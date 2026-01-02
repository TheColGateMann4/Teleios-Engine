#include "RenderGraphicsStep.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"

#include "Graphics/Bindables/IndexBuffer.h"
#include "Graphics/Bindables/VertexBuffer.h"
#include "Graphics/Bindables/ConstantBuffer.h"
#include "Graphics/Bindables/Texture.h"

RenderGraphicsStep::RenderGraphicsStep(const std::string& name)
	:
	RenderStep(name)
{

}

RenderGraphicsStep::RenderGraphicsStep()
	:
	RenderStep()
{

}

void RenderGraphicsStep::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	m_bindableContainer.Initialize(pipeline);

	m_rootSignature = std::make_unique<RootSignature>();

	// initializing root signature
	{
		{
			const auto& rootSignatureBindables = m_bindableContainer.GetRootSignatureBindables();

			for (auto& pRootSignatureBindable : rootSignatureBindables)
			{
				pRootSignatureBindable->InternalInitialize(graphics);
				pRootSignatureBindable->BindToRootSignature(graphics, m_rootSignature.get());
			}
		}

		m_rootSignature->Initialize(graphics);
	}

	// initialize pipeline state object
	{
		m_pipelineState = std::make_unique<GraphicsPipelineState>();

		// initializing pipeline state desc
		{
			{
				const auto& pipelineStateBindables = m_bindableContainer.GetPipelineStateBindables();

				for (auto& pPipelineStateBindable : pipelineStateBindables)
					pPipelineStateBindable->BindToPipelineState(graphics, m_pipelineState.get());
			}

			m_pipelineState->SetRootSignature(m_rootSignature.get());

			m_pipelineState->SetSampleMask(0xffffffff);

			m_pipelineState->SetSampleDesc(1, 0);

			m_pipelineState->SetNumRenderTargets(1);

			m_pipelineState->SetRenderTargetFormat(0, graphics.GetBackBuffer()->GetFormat());

			m_pipelineState->SetDepthStencilFormat(graphics.GetDepthStencil()->GetResource(graphics)->GetFormat());
		}

		m_pipelineState->Finish(graphics); // Finish() call gets object from desc it made up
	}

	InitializeGraphicResources(graphics, pipeline);
}


void RenderGraphicsStep::Execute(Graphics& graphics, CommandList* commandList) const
{
	commandList->SetPipelineState(graphics, m_pipelineState.get());

	commandList->SetGraphicsRootSignature(graphics, m_rootSignature.get());

	commandList->SetDescriptorHeap(graphics, &graphics.GetDescriptorHeap());

	{
		const auto& commandListBindables = m_bindableContainer.GetCommandListBindables();

		for (auto& pCommandListBindable : commandListBindables)
			pCommandListBindable->BindToCommandList(graphics, commandList);
	}

	commandList->DrawIndexed(graphics, m_bindableContainer.GetIndexBuffer()->GetIndexCount());
};

void RenderGraphicsStep::Update(Graphics& graphics, Pipeline& pipeline)
{
	m_bindableContainer.GetVertexBuffer()->BindToCopyPipelineIfNeeded(graphics, pipeline);

	m_bindableContainer.GetIndexBuffer()->BindToCopyPipelineIfNeeded(graphics, pipeline);
}

void RenderGraphicsStep::InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline)
{
	for (auto texture : m_bindableContainer.GetTextures())
		texture->InitializeGraphicResources(graphics, pipeline);
}

void RenderGraphicsStep::DrawConstantBuffers(Graphics& graphics)
{
	const std::vector<CachedConstantBuffer*>& cachedBuffers = m_bindableContainer.GetCachedBuffers();

	for (auto& cachedBuffer : cachedBuffers)
		cachedBuffer->DrawImguiProperties(graphics);
}

void RenderGraphicsStep::AddStaticBindable(const char* bindableName)
{
	m_bindableContainer.AddStaticBindable(bindableName);
}

void RenderGraphicsStep::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

const MeshBindableContainer& RenderGraphicsStep::GetBindableContainter() const
{
	return m_bindableContainer;
}