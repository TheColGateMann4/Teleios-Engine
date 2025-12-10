#include "LODMeshBindable.h"

#include "Graphics.h"
#include "Pipeline.h"

#include "Shader.h"
#include "Buffer.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"

#include "TempCommandList.h"

#include "UnorderedAccessView.h"
#include "ShaderResourceView.h"

#include "LODMesh.h"

#include <imgui.h>

void LODMeshBindable::Initialize(Graphics& graphics, Pipeline& pipeline, LODMesh* mesh)
{
	m_mesh = mesh;

	IndexBuffer* indexBuffer = mesh->GetIndexBuffer();
	VertexBuffer* vertexBuffer = mesh->GetVertexBuffer();

	for (int i = 0; i < m_LODLevels; i++)
	{
		m_vertexBufferLODs.push_back(VertexBuffer(graphics, vertexBuffer->GetLayout(), vertexBuffer->GetBuffer()->GetNumElements()));
		m_indexBufferLODs.push_back(IndexBuffer(graphics, nullptr, indexBuffer->GetBuffer()->GetNumElements(), indexBuffer->GetFormat()));

		SimplyfyMesh(graphics, pipeline, &m_vertexBufferLODs.back(), &m_indexBufferLODs.back(), i + 1);
	}
}

void LODMeshBindable::SimplyfyMesh(Graphics& graphics, Pipeline& pipeline, VertexBuffer* targetVertexBuffer, IndexBuffer* targetIndexBuffer, int lod)
{
	THROW_INTERNAL_ERROR_IF("LOD level was 0", lod == 0);

	std::string eventName = "Computing Model LOD " + std::to_string(lod);
	pipeline.GetGraphicCommandList()->BeginEvent(eventName);

	DirectX::XMUINT3 gridResolution = DirectX::XMUINT3(m_gridResolution.x / lod, m_gridResolution.y / lod, m_gridResolution.z / lod);

	Buffer* vertIn = m_mesh->GetVertexBuffer()->GetBuffer();
	Buffer* indIn = m_mesh->GetIndexBuffer()->GetBuffer();

	std::shared_ptr<Buffer> vertToCell;
	std::shared_ptr<Buffer> cellSum;
	std::shared_ptr<Buffer> cellCount;

	const unsigned int numCells = gridResolution.x * gridResolution.y * gridResolution.z;

	// assigning vertices to cells
	{

		DynamicConstantBuffer::ConstantBufferLayout layout;
		layout.AddElement<DynamicConstantBuffer::ElementType::Uint3>("gridResolution");
		layout.AddElement<DynamicConstantBuffer::ElementType::Uint>("numVertices");
		layout.AddElement<DynamicConstantBuffer::ElementType::Uint>("numCells");

		DynamicConstantBuffer::ConstantBufferData bufferData(layout);
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Uint3>("gridResolution") = gridResolution;
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Uint>("numVertices") = vertIn->GetNumElements();
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Uint>("numCells") = numCells;

		std::shared_ptr<TempConstantBuffer> modelInfo = std::make_shared<TempConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{ {ShaderVisibilityGraphic::AllShaders, 0} });
		cellSum = std::make_shared<Buffer>(graphics, sizeof(float), 3 * numCells, Buffer::CPUAccess::notavailable, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		cellCount = std::make_shared<Buffer>(graphics, sizeof(int), numCells, Buffer::CPUAccess::notavailable, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		vertToCell = std::make_shared<Buffer>(graphics, sizeof(int), vertIn->GetNumElements(), Buffer::CPUAccess::notavailable, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		static const std::shared_ptr<Shader> assignCS = Shader::GetBindableResource(graphics, L"CS_AssignVertices", ShaderType::ComputeShader);
		Buffer* outMin = m_mesh->GetBoundaryBoxMinGPUResource();
		Buffer* outMax = m_mesh->GetBoundaryBoxMaxGPUResource();

		TempComputeCommandList computeCommandList(graphics, pipeline.GetGraphicCommandList());

		{
			ShaderResourceView vertInSRV(graphics, vertIn, 0);
			ShaderResourceView outMinSRV(graphics, outMin, 1);
			ShaderResourceView outMaxSRV(graphics, outMax, 2);
			UnorderedAccessView cellSumUAV(graphics, cellSum.get(), 0);
			UnorderedAccessView cellCountUAV(graphics, cellCount.get(), 1);
			UnorderedAccessView vertToCellUAV(graphics, vertToCell.get(), 2);

			computeCommandList.Bind(assignCS);
			computeCommandList.Bind(modelInfo); // b0
			computeCommandList.Bind(std::move(vertInSRV)); // t0
			computeCommandList.Bind(std::move(outMinSRV)); // t1
			computeCommandList.Bind(std::move(outMaxSRV)); // t2
			computeCommandList.Bind(std::move(cellSumUAV)); // u0
			computeCommandList.Bind(std::move(cellCountUAV)); // u1
			computeCommandList.Bind(std::move(vertToCellUAV)); // u2

			computeCommandList.Dispatch(graphics, vertIn->GetNumElements());
		}

		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(computeCommandList));

		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, cellSum);
		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, cellCount);
		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, vertToCell);
	}

	// taking avg of vertices
	{
		Buffer* vertOut = targetVertexBuffer->GetBuffer();
		static const std::shared_ptr<Shader> avgCS = Shader::GetBindableResource(graphics, L"CS_AverageVertices", ShaderType::ComputeShader);

		TempComputeCommandList computeCommandList(graphics, pipeline.GetGraphicCommandList());

		{
			UnorderedAccessView cellSumUAV(graphics, cellSum.get(), 0);
			UnorderedAccessView cellCountUAV(graphics, cellCount.get(), 1);
			UnorderedAccessView vertOutUAV(graphics, vertOut, 2);

			computeCommandList.Bind(avgCS);
			computeCommandList.Bind(std::move(cellCountUAV)); // u0
			computeCommandList.Bind(std::move(cellSumUAV)); // u1
			computeCommandList.Bind(std::move(vertOutUAV)); // u2

			computeCommandList.Dispatch(graphics, numCells);
		}

		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(computeCommandList));
	}

	// building indices stage
	{
		Buffer* indOut = targetIndexBuffer->GetBuffer();
		static const std::shared_ptr<Shader> buildCS = Shader::GetBindableResource(graphics, L"CS_BuildIndices", ShaderType::ComputeShader);

		TempComputeCommandList computeCommandList(graphics, pipeline.GetGraphicCommandList());

		{
			ShaderResourceView indInSRV(graphics, indIn);
			ShaderResourceView vertToCellUAV(graphics, vertToCell.get(), 1);
			UnorderedAccessView indOutUAV(graphics, indOut, 0);

			computeCommandList.Bind(buildCS);
			computeCommandList.Bind(std::move(indInSRV)); // t0
			computeCommandList.Bind(std::move(vertToCellUAV)); // t1
			computeCommandList.Bind(std::move(indOutUAV)); // u0

			computeCommandList.Dispatch(graphics, indIn->GetNumElements() / 3);
		}

		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(computeCommandList));
	}
	pipeline.GetGraphicCommandList()->EndEvent();

}

void LODMeshBindable::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetVertexBuffer(graphics, GetVertexBuffer(GetLODLevel(graphics)));

	commandList->SetIndexBuffer(graphics, GetIndexBuffer(GetLODLevel(graphics)));
}

void LODMeshBindable::SetLODLevel(int lodLevel)
{
	//m_LODLevels = lodLevel;
}

VertexBuffer* LODMeshBindable::GetVertexBuffer(int lod)
{
	if (lod == 0)
		return m_mesh->GetVertexBuffer();

	return &m_vertexBufferLODs.at(lod);
}

IndexBuffer* LODMeshBindable::GetIndexBuffer(int lod)
{
	if (lod == 0)
		return m_mesh->GetIndexBuffer();

	return &m_indexBufferLODs.at(lod);
}

int LODMeshBindable::GetLODLevel(Graphics& graphics)
{
	// TODO: Use screen resolution, distance, and object sizes determine what LOD to choose

	static int m_currentLOD = 0;
	static bool alreadyPressedPlus = false;
	static bool alreadyPressedMinus = false;
	
	bool holdsPlus = GetAsyncKeyState(VK_ADD) & 0x8000;
	bool holdsMinus = GetAsyncKeyState(VK_SUBTRACT) & 0x8000;
	
	if (holdsPlus != alreadyPressedPlus)
	{
		if (!alreadyPressedPlus && holdsPlus)
			if(m_currentLOD != m_LODLevels - 1)
				m_currentLOD++;
	
		alreadyPressedPlus = holdsPlus;
	}
	
	if (holdsMinus != alreadyPressedMinus)
	{
		if (!alreadyPressedMinus && holdsMinus)
			if(m_currentLOD != 0)
				m_currentLOD--;
	
		alreadyPressedMinus = holdsMinus;
	}
	std::cout << m_currentLOD << '\n';

	return m_currentLOD;
}