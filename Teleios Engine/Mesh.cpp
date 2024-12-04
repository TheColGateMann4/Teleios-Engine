#include "Mesh.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "Camera.h"


Mesh::Mesh(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation)
	:
	m_position(position),
	m_rotation(rotation)
{

}

void Mesh::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	for (auto staticBindableName : m_staticBindableNames)
		SegregateBindableOnStartingPos(pipeline.GetStaticResource(staticBindableName));

	m_rootSignature = std::make_unique<RootSignature>();

	// initializing root signature
	{
		for (auto& pRootSignatureBindable : m_rootSignatureBindables)
		{
			pRootSignatureBindable->InternalInitialize(graphics);
			pRootSignatureBindable->BindToRootSignature(graphics, m_rootSignature.get());
		}

		m_transformConstantBuffer->SetParentPtr(this);

		m_rootSignature->Initialize(graphics);
	}

	// initialize pipeline state object
	{
		m_pipelineState = std::make_unique<GraphicsPipelineState>();

		// initializing pipeline state desc
		{
			for (auto& pPipelineStateBindable : m_pipelineStateBindables)
				pPipelineStateBindable->BindToPipelineState(graphics, m_pipelineState.get());

			m_pipelineState->SetRootSignature(m_rootSignature.get());

			m_pipelineState->SetSampleMask(0xffffffff);

			m_pipelineState->SetSampleDesc(1, 0);

			m_pipelineState->SetNumRenderTargets(1);

			m_pipelineState->SetRenderTargetFormat(0, graphics.GetBackBuffer()->GetFormat());

			m_pipelineState->SetDepthStencilFormat(graphics.GetDepthStencil()->GetFormat());
		}

		m_pipelineState->Finish(graphics); // Finish() call gets object from desc it made up
	}
}

void Mesh::DrawMesh(Graphics& graphics, Pipeline& pipeline) const
{
	CommandList* directCommandList = pipeline.GetGraphicCommandList();

	directCommandList->SetPipelineState(graphics, m_pipelineState.get());

	for (auto& pDirectCommandListBindable : m_directCommandListBindables)
		pDirectCommandListBindable->BindToDirectCommandList(graphics, directCommandList);

	directCommandList->SetGraphicsRootSignature(graphics, m_rootSignature.get());

	directCommandList->SetDescriptorHeap(graphics, &graphics.GetDescriptorHeap());

	for (auto& pCommandListBindable : m_commandListBindables)
		pCommandListBindable->BindToCommandList(graphics, directCommandList);

	directCommandList->DrawIndexed(graphics, m_indexBuffer->GetIndexCount());
};

void Mesh::SetPosition(DirectX::XMFLOAT3 position)
{
	m_position = position;
}

DirectX::XMFLOAT3& Mesh::GetPositionLVal()
{
	return m_position;
}

DirectX::XMFLOAT3& Mesh::GetRotationLVal()
{
	return m_rotation;
}

void Mesh::InternalUpdate(Graphics& graphics)
{

}

void Mesh::UpdateTransformMatrix(Graphics& graphics, Camera& camera)
{
	if(m_transformChanged || camera.ViewChanged())
		m_transformConstantBuffer->Update(graphics, camera);
}

DirectX::XMMATRIX Mesh::GetTransformMatrix() const
{
	DirectX::FXMVECTOR vecPosition = DirectX::XMLoadFloat3(&m_position);
	DirectX::FXMVECTOR vecRotation = DirectX::XMLoadFloat3(&m_rotation);

	//  multiplying position matrix by rotation matrix
	return DirectX::XMMatrixRotationRollPitchYawFromVector(vecRotation) * DirectX::XMMatrixTranslationFromVector(vecPosition);
}


bool Mesh::TransformChanged() const
{
	return m_transformChanged;
}

void Mesh::SetTransformChanged(bool val)
{
	m_transformChanged = val;
}

void Mesh::AddStaticBindable(const char* bindableName)
{
	m_staticBindableNames.push_back(bindableName);
}

void Mesh::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindables.push_back(bindable);

	SegregateBindable(bindable.get());
}

void Mesh::SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer)
{
	AddBindable(vertexBuffer);

	m_vertexBuffer = vertexBuffer.get();
}

void Mesh::SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer)
{
	AddBindable(indexBuffer);

	m_indexBuffer = indexBuffer.get();
}

void Mesh::SetTransformConstantBuffer(std::shared_ptr<TransformConstantBuffer> transformConstantBuffer)
{
	AddBindable(transformConstantBuffer);

	m_transformConstantBuffer = transformConstantBuffer.get();
}

void Mesh::SegregateBindable(Bindable* bindable)
{
	if (auto commandListBindable = dynamic_cast<CommandListBindable*>(bindable))
		m_commandListBindables.push_back(commandListBindable);

	if (auto directCommandListBindable = dynamic_cast<DirectCommandListBindable*>(bindable))
		m_directCommandListBindables.push_back(directCommandListBindable);

	if (auto rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(bindable))
		m_rootSignatureBindables.push_back(rootSignatureBindable);

	if (auto pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(bindable))
		m_pipelineStateBindables.push_back(pipelineStateBindable);
}

void Mesh::SegregateBindableOnStartingPos(Bindable* bindable)
{
	if (auto commandListBindable = dynamic_cast<CommandListBindable*>(bindable))
		m_commandListBindables.insert(m_commandListBindables.begin(), commandListBindable);

	if (auto directCommandListBindable = dynamic_cast<DirectCommandListBindable*>(bindable))
		m_directCommandListBindables.insert(m_directCommandListBindables.begin(), directCommandListBindable);

	if (auto rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(bindable))
		m_rootSignatureBindables.insert(m_rootSignatureBindables.begin(), rootSignatureBindable);

	if (auto pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(bindable))
		m_pipelineStateBindables.insert(m_pipelineStateBindables.begin(), pipelineStateBindable);
}

VertexBuffer* Mesh::GetVertexBuffer()
{
	return m_vertexBuffer;
}

IndexBuffer* Mesh::GetIndexBuffer()
{
	return m_indexBuffer;
}