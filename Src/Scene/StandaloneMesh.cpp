#include "StandaloneMesh.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Scene/Objects/Camera.h"

void StandaloneMesh::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	m_bindableContainer.Initialize(pipeline);

	// initializing root signature
	{
		RootSignatureParams rootParams = {};
		{
			for (auto& descriptorBindable : m_bindableContainer.GetDescriptorBindables())
				descriptorBindable->Initialize(graphics);

			for (auto& rootSignatureBindable : m_bindableContainer.GetRootSignatureBindables())
				rootSignatureBindable->BindToRootSignature(&rootParams);
		}

		m_rootSignature = RootSignature::GetResource(graphics, std::move(rootParams));
	}

	// initialize pipeline state object
	{
		GraphicsPipelineStateParams pipelineStateParams = {};
		{
			{
				const auto& pipelineStateBindables = m_bindableContainer.GetPipelineStateBindables();

				for (auto& pPipelineStateBindable : pipelineStateBindables)
					pPipelineStateBindable->AddPipelineStateParam(graphics, &pipelineStateParams);
			}

			pipelineStateParams.SetRootSignature(m_rootSignature.get());

			pipelineStateParams.SetSampleMask(0xffffffff);

			pipelineStateParams.SetSampleDesc(1, 0);

			pipelineStateParams.SetNumRenderTargets(1);

			pipelineStateParams.SetRenderTargetFormat(0, graphics.GetBackBuffer()->GetFormat());

			pipelineStateParams.SetDepthStencilFormat(graphics.GetDepthStencil()->GetResource(graphics)->GetFormat());
		}

		m_pipelineState = GraphicsPipelineState::GetResource(graphics, std::move(pipelineStateParams));
	}

	for (auto* cachedBuffer : m_bindableContainer.GetCachedBuffers())
		cachedBuffer->Update(graphics);
}

void StandaloneMesh::Draw(Graphics& graphics, CommandList* commandList) const
{
	commandList->SetPipelineState(graphics, m_pipelineState.get());

	commandList->SetGraphicsRootSignature(graphics, m_rootSignature.get());

	{
		const auto& commandListBindables = m_bindableContainer.GetCommandListBindables();

		for (auto& pCommandListBindable : commandListBindables)
			pCommandListBindable->BindToCommandList(graphics, commandList);
	}

	commandList->DrawIndexed(graphics, m_bindableContainer.GetIndexBuffer()->GetIndexCount());
};

void StandaloneMesh::Update(Graphics& graphics, Pipeline& pipeline)
{

}

void StandaloneMesh::InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline)
{
	for (auto texture : m_bindableContainer.GetTextures())
		texture->InitializeGraphicResources(graphics, pipeline);
}

void StandaloneMesh::DrawConstantBuffers(Graphics& graphics)
{
	const std::vector<CachedConstantBuffer*>& cachedBuffers = m_bindableContainer.GetCachedBuffers();

	for (auto& cachedBuffer : cachedBuffers)
		cachedBuffer->DrawImguiProperties(graphics);
}

void StandaloneMesh::AddStaticBindable(const char* bindableName)
{
	m_bindableContainer.AddStaticBindable(bindableName);
}

void StandaloneMesh::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

const MeshBindableContainer& StandaloneMesh::GetBindableContainter() const
{
	return m_bindableContainer;
}