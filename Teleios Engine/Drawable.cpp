#include "Drawable.h"
#include "Graphics.h"
#include "Pipeline.h"


Drawable::Drawable(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation)
	:
	m_position(position),
	m_rotation(rotation)
{

}

void Drawable::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	for (auto staticBindableName : m_staticBindableNames)
		SegregateBindableOnStartingPos(pipeline.GetStaticResource(staticBindableName));

	//m_bundleCommandList = std::make_unique<CommandList>(graphics, D3D12_COMMAND_LIST_TYPE_BUNDLE);

	m_rootSignature = std::make_unique<RootSignature>();

	// initializing root signature
	{
		for (auto& pRootSignatureBindable : m_rootSignatureBindables)
		{
			pRootSignatureBindable->InternalInitialize(graphics);
			pRootSignatureBindable->BindToRootSignature(graphics, m_rootSignature.get());
		}

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

	RecordBundleList(graphics);
}

void Drawable::RecordBundleList(Graphics& graphics)
{
//	THROW_OBJECT_STATE_ERROR_IF("BundleList was not yet initialized", !m_bundleCommandList);
//	THROW_OBJECT_STATE_ERROR_IF("PipelineState was not yet initialized", !m_pipelineState);
//	THROW_OBJECT_STATE_ERROR_IF("RootSignature was not yet initialized", !m_rootSignature);
//
//	m_bundleCommandList->Open(graphics, m_pipelineState.get());
//
//	m_bundleCommandList->SetRootSignature(graphics, m_rootSignature.get());
//
//	for (auto& pCommandListBindable : m_commandListBindables)
//		pCommandListBindable->BindToCommandList(graphics, m_bundleCommandList.get());
//
//	m_bundleCommandList->DrawIndexed(graphics, m_indexBuffer->GetIndexCount());
//
//	m_bundleCommandList->Close(graphics);
}

void Drawable::DrawDrawable(Graphics& graphics, Pipeline& pipeline) const
{
	CommandList* directCommandList = pipeline.GetGraphicCommandList();

	directCommandList->SetPipelineState(graphics, m_pipelineState.get());

	for (auto& pDirectCommandListBindable : m_directCommandListBindables)
		pDirectCommandListBindable->BindToDirectCommandList(graphics, directCommandList);

	//	directCommandList->ExecuteBundle(graphics, m_bundleCommandList.get());

	directCommandList->SetGraphicsRootSignature(graphics, m_rootSignature.get());

	directCommandList->SetDescriptorHeap(graphics, &graphics.GetDescriptorHeap());

	for (auto& pCommandListBindable : m_commandListBindables)
		pCommandListBindable->BindToCommandList(graphics, directCommandList);

	directCommandList->DrawIndexed(graphics, m_indexBuffer->GetIndexCount());
};

void Drawable::SetPosition(DirectX::XMFLOAT3 position)
{
	m_position = position;
}

void Drawable::InternalUpdate(Graphics& graphics)
{

}

void Drawable::UpdateTransformMatrix(Graphics& graphics, Camera& camera)
{
	m_transformConstantBuffer->Update(graphics, camera);
}

DirectX::XMMATRIX Drawable::GetTransformMatrix() const
{
	DirectX::FXMVECTOR vecPosition = DirectX::XMLoadFloat3(&m_position);
	DirectX::FXMVECTOR vecRotation = DirectX::XMLoadFloat3(&m_rotation);

	//  multiplying position matrix by rotation matrix
	return DirectX::XMMatrixRotationRollPitchYawFromVector(vecRotation) * DirectX::XMMatrixTranslationFromVector(vecPosition);
}

void Drawable::AddStaticBindable(const char* bindableName)
{
	m_staticBindableNames.push_back(bindableName);
}

void Drawable::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindables.push_back(bindable);

	SegregateBindable(bindable.get());
}

void Drawable::SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer)
{
	AddBindable(vertexBuffer);

	m_vertexBuffer = vertexBuffer.get();
}

void Drawable::SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer)
{
	AddBindable(indexBuffer);

	m_indexBuffer = indexBuffer.get();
}

void Drawable::SetTransformConstantBuffer(std::shared_ptr<TransformConstantBuffer> transformConstantBuffer)
{
	AddBindable(transformConstantBuffer);

	m_transformConstantBuffer = transformConstantBuffer.get();
}

void Drawable::SegregateBindable(Bindable* bindable)
{
	// we are pushing cached buffer here since we don't want static bindables to be updated by each object on scene
	if (auto cachedBuffer = dynamic_cast<CachedConstantBuffer*>(bindable))
	{
		m_cachedBuffers.push_back(cachedBuffer);

		// saving some processing
		m_commandListBindables.push_back(cachedBuffer);
		m_rootSignatureBindables.push_back(cachedBuffer);

		return;
	}


	if (auto commandListBindable = dynamic_cast<CommandListBindable*>(bindable))
		m_commandListBindables.push_back(commandListBindable);

	if (auto directCommandListBindable = dynamic_cast<DirectCommandListBindable*>(bindable))
		m_directCommandListBindables.push_back(directCommandListBindable);

	if (auto rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(bindable))
		m_rootSignatureBindables.push_back(rootSignatureBindable);

	if (auto pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(bindable))
		m_pipelineStateBindables.push_back(pipelineStateBindable);
}

void Drawable::SegregateBindableOnStartingPos(Bindable* bindable)
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