#include "LODMesh.h"
#include "includes\CppIncludes.h"

#include "Graphics.h"
#include "Pipeline.h"

#include "TempCommandList.h"

#include "Shader.h"
#include "UnorderedAccessView.h"
#include "ShaderResourceView.h"

LODMesh::LODMesh()
	:
	m_lodMeshBindable(std::make_shared<LODMeshBindable>())
{
	m_bindableContainer.AddBindable(m_lodMeshBindable);
}

void LODMesh::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	// current binding system will be rewriten to allow GPU to make stuff in paralel and transition resource states in bulk
	// thus we just gonna do it like this for now
	{
		GetVertexBuffer()->BindToCopyPipelineIfNeeded(graphics, pipeline);
		GetIndexBuffer()->BindToCopyPipelineIfNeeded(graphics, pipeline);

		pipeline.ExecuteCopyCalls(graphics);

		pipeline.GetGraphicCommandList()->SetResourceState(graphics, GetVertexBuffer()->GetBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		pipeline.GetGraphicCommandList()->SetResourceState(graphics, GetIndexBuffer()->GetBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	GetModelBounds(graphics, pipeline);

	m_lodMeshBindable->Initialize(graphics, pipeline, this);

	{
		pipeline.GetGraphicCommandList()->SetResourceState(graphics, GetVertexBuffer()->GetBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		pipeline.GetGraphicCommandList()->SetResourceState(graphics, GetIndexBuffer()->GetBuffer(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	Mesh::Initialize(graphics, pipeline);
}

// we are overriding this function since here we set local pointer to most detalied vertexBuffer, 
// but we don't bind it to rest of the pipeline. To pipeline we will bind LODMeshBindable that contains Vertex and Index buffers and chooses right LOD
void LODMesh::SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer)
{
	m_vertexBuffer = vertexBuffer.get();
}

void LODMesh::SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer)
{
	m_indexBuffer = indexBuffer.get();
}

Buffer* LODMesh::GetBoundaryBoxMinGPUResource() const
{
	return m_boundaryBoxMin.get();
}

Buffer* LODMesh::GetBoundaryBoxMaxGPUResource() const
{
	return m_boundaryBoxMax.get();
}

void LODMesh::GetModelBounds(Graphics& graphics, Pipeline& pipeline)
{
	BEGIN_COMMAND_LIST_EVENT(pipeline.GetGraphicCommandList(), "Computing Model Bounds");

	Buffer* vertIn = GetVertexBuffer()->GetBuffer();
	
	DynamicConstantBuffer::ConstantBufferLayout layout;
	layout.AddElement<DynamicConstantBuffer::ElementType::Uint>("fieldOffsetOfVertices");
	layout.AddElement<DynamicConstantBuffer::ElementType::Uint>("numVertices");

	DynamicConstantBuffer::ConstantBufferData bufferData(layout);
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Uint>("fieldOffsetOfVertices") = int(GetVertexBuffer()->GetLayout().GetElementOffset<DynamicVertex::ElementType::Position>());
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Uint>("numVertices") = vertIn->GetNumElements();

	std::shared_ptr<TempConstantBuffer> modelInfo = std::make_shared<TempConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{ {ShaderVisibilityGraphic::AllShaders, 0} });

	m_boundaryBoxMin = std::make_shared<Buffer>(graphics, sizeof(float), 3, Buffer::CPUAccess::readwrite, D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	m_boundaryBoxMax = std::make_shared<Buffer>(graphics, sizeof(float), 3, Buffer::CPUAccess::readwrite, D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	static const std::shared_ptr<Shader> computeShader = Shader::GetBindableResource(graphics, L"CS_ModelBounds", ShaderType::ComputeShader);

	TempComputeCommandList computeCommandList(graphics, pipeline.GetGraphicCommandList());

	{
		ShaderResourceView vertInSRV(graphics, vertIn, 0);
		UnorderedAccessView outMinUAV(graphics, m_boundaryBoxMin.get(), 0);
		UnorderedAccessView outMaxUAV(graphics, m_boundaryBoxMax.get(), 1);

		computeCommandList.Bind(computeShader);
		computeCommandList.Bind(std::move(vertInSRV)); //t0
		computeCommandList.Bind(modelInfo); // b0
		computeCommandList.Bind(std::move(outMinUAV)); // u0
		computeCommandList.Bind(std::move(outMaxUAV)); // u1

		computeCommandList.Dispatch(graphics, vertIn->GetNumElements());
	}

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(computeCommandList));

	END_COMMAND_LIST_EVENT(pipeline.GetGraphicCommandList());
}