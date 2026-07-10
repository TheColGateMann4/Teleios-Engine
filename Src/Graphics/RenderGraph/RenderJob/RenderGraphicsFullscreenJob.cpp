#include "RenderGraphicsFullscreenJob.h"
#include "Graphics/RenderGraph/RenderPass/RenderPass.h"
#include "Graphics/RenderGraph/Steps/RenderGraphicsFullscreenStep.h"
#include "Graphics/RenderGraph/RenderPass/Fullscreen/FullscreenPass.h"

#include "Graphics/Core/CommandList.h"

#include "Graphics/Core/Pix.h"

RenderGraphicsFullscreenJob::RenderGraphicsFullscreenJob(GraphicsRenderData renderData, FullscreenPass* pass)
	:
	GraphicsRenderJob(renderData.type),
	m_step(static_cast<RenderGraphicsFullscreenStep*>(renderData.step)),
	m_pass(pass)
{

}

void RenderGraphicsFullscreenJob::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	if (m_initialized)
		return;
	m_initialized = true;

	BuildRootSignature(graphics);

	BuildPipelineState(graphics);

	InitializeGraphicResources(graphics, pipeline);
}

void RenderGraphicsFullscreenJob::InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline)
{
	const auto& stepBindableContainer = m_step->GetBindableContainer();

	auto attributeVertexEntry = stepBindableContainer.GetAttributeVertexBufferEntry();
	auto positionVertexEntry = stepBindableContainer.GetPositionVertexBufferEntry();
	auto indexBuffer = stepBindableContainer.GetIndexBufferEntry();

	THROW_INTERNAL_ERROR_IF("None vertex buffer was bound", !attributeVertexEntry && !positionVertexEntry);
	THROW_INTERNAL_ERROR_IF("Index buffer hasn't been bound", !indexBuffer);
}

void RenderGraphicsFullscreenJob::Execute(Graphics& graphics, CommandList* commandList) const
{
	START_CPU_EVENT(PIX_COLOR(0, 127, 127), "DrawIndexed");

	commandList->SetPipelineState(graphics, m_pipelineState.get());

	commandList->SetGraphicsRootSignature(graphics, m_rootSignature.get());

	const auto& stepBindableContainer = m_step->GetBindableContainer();

	std::shared_ptr<IndexBufferEntry> indexBufferEntry = stepBindableContainer.GetIndexBufferEntry();
	std::shared_ptr<VertexBufferEntry> vertexBufferEntry;
	{
		auto attribBufferEntry = stepBindableContainer.GetAttributeVertexBufferEntry();
		auto positionBufferEntry = stepBindableContainer.GetPositionVertexBufferEntry();

		if (attribBufferEntry)
			vertexBufferEntry = attribBufferEntry;
		else if (positionBufferEntry)
			vertexBufferEntry = positionBufferEntry;
		else
		{
			THROW_INTERNAL_ERROR("Position buffer and Attribute buffer were both NULL");
		}
	}

	{
		m_rootSignatureLayout.BindToCommandList(graphics, commandList);

		for (auto& pCommandListBindable : stepBindableContainer.GetCommandListBindables())
			pCommandListBindable->BindToCommandList(graphics, commandList);

		vertexBufferEntry->BindToCommandList(graphics, commandList);
		indexBufferEntry->BindToCommandList(graphics, commandList);
	}

	unsigned int indices = stepBindableContainer.GetIndexBufferEntry()->GetIndexCount();
	unsigned int baseVertexOffset = vertexBufferEntry->GetEntryInfo()->elementOffset;
	unsigned int startIndexOffset = indexBufferEntry->GetEntryInfo()->elementOffset;

	commandList->DrawIndexed(graphics, indices, baseVertexOffset, startIndexOffset);

	END_CPU_EVENT();
}

void RenderGraphicsFullscreenJob::BuildRootSignature(Graphics& graphics)
{
	RootSignatureParams rootParams = {};
	{
		{
			const auto& stepBindableContainer = m_step->GetBindableContainer();

			for (auto& descriptorBindable : stepBindableContainer.GetDescriptorBindables())
				descriptorBindable->Initialize(graphics);

			for (auto& rootSignatureBindable : stepBindableContainer.GetRootSignatureBindables())
				rootSignatureBindable->AddGraphicsRootSignatureParam(&rootParams);
		}
	}

	m_rootSignatureLayout = rootParams.GetLayout();
	m_rootSignature = RootSignature::GetResource(graphics, std::move(rootParams));
}

void RenderGraphicsFullscreenJob::BuildPipelineState(Graphics& graphics)
{
	const std::vector<RenderPass::RenderTargetData>& renderTargets = m_pass->GetRenderTargets();
	RenderPass::DepthStencilData depthStencilView = m_pass->GetDepthStencilView();

	GraphicsPipelineStateParams pipelineStateParams = {};

	{
		for (auto& pPipelineStateBindable : m_step->GetBindableContainer().GetPipelineStateBindables())
			pPipelineStateBindable->AddPipelineStateParam(graphics, &pipelineStateParams);

		pipelineStateParams.SetRootSignature(m_rootSignature.get());

		pipelineStateParams.SetSampleMask(0xffffffff);

		pipelineStateParams.SetSampleDesc(1, 0);

		pipelineStateParams.SetNumRenderTargets(renderTargets.size());

		for (int i = 0; i < renderTargets.size(); i++)
			pipelineStateParams.SetRenderTargetFormat(i, renderTargets.at(i).resource->GetFormat());

		pipelineStateParams.SetDepthStencilFormat(depthStencilView.resource ? depthStencilView.resource->GetResource(graphics)->GetFormat() : DXGI_FORMAT_UNKNOWN);
	}

	m_pipelineState = GraphicsPipelineState::GetResource(graphics, std::move(pipelineStateParams));
}