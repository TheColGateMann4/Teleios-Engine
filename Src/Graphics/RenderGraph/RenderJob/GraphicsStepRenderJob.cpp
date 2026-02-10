#include "GraphicsStepRenderJob.h"

#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"
#include "Graphics/RenderGraph/RenderPass/Geometry/GeometryPass.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/CommandList.h"

#include "Graphics/Bindables/VertexBuffer.h"
#include "Graphics/Bindables/IndexBuffer.h"
#include "Graphics/Bindables/Texture.h"

GraphicsStepRenderJob::GraphicsStepRenderJob(RenderJob::JobType m_type, RenderGraphicsStep* step)
	:
	StepRenderJob(m_type),
	m_step(step)
{

}

void GraphicsStepRenderJob::GatherBindables()
{
	THROW_INTERNAL_ERROR_IF("Pass was not linked to job", m_pass == nullptr);

	const auto& passBinds = m_pass->GetBindables();

	for (const auto& passBind : passBinds)
		m_bindableContainer.AddBindable(passBind);
	
	// appending step bindables to local container
	m_bindableContainer += m_step->GetBindableContainter();
}

void GraphicsStepRenderJob::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	m_bindableContainer.Initialize(pipeline);

	m_rootSignature = std::make_unique<RootSignature>();

	const std::vector<RenderPass::RenderTargetData>& renderTargets = m_pass->GetRenderTargets();
	RenderPass::DepthStencilData depthStencilView = m_pass->GetDepthStencilView();

	// initializing root signature
	{
		{
			auto& rootSignatureBindables = m_bindableContainer.GetRootSignatureBindables();

			for (auto& rootSignatureBindable : rootSignatureBindables)
			{
				rootSignatureBindable->Initialize(graphics);
				rootSignatureBindable->BindToRootSignature(m_rootSignature.get());
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

			m_pipelineState->SetNumRenderTargets(renderTargets.size());

			for(int i = 0; i < renderTargets.size(); i++)
				m_pipelineState->SetRenderTargetFormat(i, renderTargets.at(i).resource->GetFormat());

			m_pipelineState->SetDepthStencilFormat(depthStencilView.resource ? depthStencilView.resource->GetResource(graphics)->GetFormat() : DXGI_FORMAT_UNKNOWN);
		}

		m_pipelineState->Finish(graphics); // Finish() call gets object from desc it made up
	}

	InitializeGraphicResources(graphics, pipeline);
}

void GraphicsStepRenderJob::InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline)
{
	m_bindableContainer.GetVertexBuffer()->BindToCopyPipelineIfNeeded(graphics, pipeline);
	m_bindableContainer.GetIndexBuffer()->BindToCopyPipelineIfNeeded(graphics, pipeline);

	for (auto texture : m_bindableContainer.GetTextures())
		texture->InitializeGraphicResources(graphics, pipeline);
}

void GraphicsStepRenderJob::Execute(Graphics& graphics, CommandList* commandList) const
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
}

RenderJob::JobGroup GraphicsStepRenderJob::GetGroup() const
{
	return JobGroup::Geometry;
}