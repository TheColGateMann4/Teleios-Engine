#include "GraphicsStepRenderJob.h"

#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"
#include "Graphics/RenderGraph/RenderPass/Geometry/GeometryPass.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/CommandList.h"

#include "Includes/BindablesInclude.h"

#include "Scene/Material.h"

GraphicsStepRenderJob::GraphicsStepRenderJob(GraphicsRenderData renderData, GeometryPass* pass)
	:
	StepRenderJob(renderData.type),
	m_step(renderData.step)
{
	m_pass = pass;
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
	InitializeMaterialBindings();

	m_bindableContainer.Initialize(pipeline);

	const std::vector<RenderPass::RenderTargetData>& renderTargets = m_pass->GetRenderTargets();
	RenderPass::DepthStencilData depthStencilView = m_pass->GetDepthStencilView();

	Material* material = m_step->GetMaterial();

	// initializing root signature
	{
		RootSignatureParams rootParams = {};
		{
			for (auto& descriptorBindable : m_bindableContainer.GetDescriptorBindables())
				descriptorBindable->Initialize(graphics);

			for (auto& rootSignatureBindable : m_bindableContainer.GetRootSignatureBindables())
				rootSignatureBindable->BindToRootSignature(&rootParams);

			if(material)
				material->BindToRootSignature(&rootParams);
		}

		m_rootSignature = RootSignature::GetResource(graphics, std::move(rootParams));
	}

	// initialize pipeline state object
	{
		GraphicsPipelineStateParams pipelineStateParams = {};
		
		{
			for (auto& pPipelineStateBindable : m_bindableContainer.GetPipelineStateBindables())
				pPipelineStateBindable->AddPipelineStateParam(graphics, &pipelineStateParams);

			if (material)
				for (auto& pPipelineStateBindable : material->GetBindableContainer().GetPipelineStateBindables())
					pPipelineStateBindable->AddPipelineStateParam(graphics, &pipelineStateParams);

			pipelineStateParams.SetRasterizerState(BuildAndGetRasterizerState(graphics, material));

			pipelineStateParams.SetRootSignature(m_rootSignature.get());

			pipelineStateParams.SetSampleMask(0xffffffff);

			pipelineStateParams.SetSampleDesc(1, 0);

			pipelineStateParams.SetNumRenderTargets(renderTargets.size());

			for(int i = 0; i < renderTargets.size(); i++)
				pipelineStateParams.SetRenderTargetFormat(i, renderTargets.at(i).resource->GetFormat());

			pipelineStateParams.SetDepthStencilFormat(depthStencilView.resource ? depthStencilView.resource->GetResource(graphics)->GetFormat() : DXGI_FORMAT_UNKNOWN);
		}

		m_pipelineState = GraphicsPipelineState::GetResource(graphics, std::move(pipelineStateParams));
	}

	InitializeGraphicResources(graphics, pipeline);
}

void GraphicsStepRenderJob::InitializeMaterialBindings()
{
	const auto* material = m_step->GetMaterial();
	const auto& bindableContainer = material ? material->GetBindableContainer() : m_bindableContainer;
	const auto& textures = bindableContainer.GetTextures();

	if (textures.empty())
		return;

	m_materialBindings = std::make_shared<MaterialBindings>(textures);

	m_bindableContainer.AddBindable(m_materialBindings->GetDescriptorHeapBindable());
	m_bindableContainer.AddBindable(m_materialBindings->GetTextureIndexesConstants());
}

void GraphicsStepRenderJob::InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline)
{
	m_bindableContainer.GetVertexBufferEntry()->GetVertexBuffer()->BindToCopyPipelineIfNeeded(graphics, pipeline);
	m_bindableContainer.GetIndexBuffer()->BindToCopyPipelineIfNeeded(graphics, pipeline);

	for (auto texture : m_bindableContainer.GetTextures())
		texture->InitializeGraphicResources(graphics, pipeline);

	for (auto* cachedBuffer : m_bindableContainer.GetCachedBuffers())
		cachedBuffer->Update(graphics);

	auto* material = m_step->GetMaterial();

	if (material)
		material->InitializeGraphicResources(graphics, pipeline);
}

void GraphicsStepRenderJob::Execute(Graphics& graphics, CommandList* commandList) const
{
	commandList->SetPipelineState(graphics, m_pipelineState.get());

	commandList->SetGraphicsRootSignature(graphics, m_rootSignature.get());

	{
		auto* material = m_step->GetMaterial();

		if (material)
			material->Bind(graphics, commandList);
	}

	{
		const auto& commandListBindables = m_bindableContainer.GetCommandListBindables();

		for (auto& pCommandListBindable : commandListBindables)
			pCommandListBindable->BindToCommandList(graphics, commandList);
	}

	unsigned int indexCount = m_bindableContainer.GetIndexBuffer()->GetIndexCount();
	unsigned int baseVertexOffset = m_bindableContainer.GetVertexBufferEntry()->GetEntryInfo().offset;

	commandList->DrawIndexed(graphics, indexCount, baseVertexOffset);
}

RenderJob::JobGroup GraphicsStepRenderJob::GetGroup() const
{
	return JobGroup::Geometry;
}

RenderGraphicsStep* GraphicsStepRenderJob::GetStep() const
{
	return m_step;
}

RasterizerState* GraphicsStepRenderJob::BuildAndGetRasterizerState(Graphics& graphics, Material* material)
{
	ObjectRasterizerStateOptions objectRasterizerOptions = material ? material->GetRasterizerOptions() : m_step->GetRasterizerOptions();

	std::shared_ptr<RasterizerState> rasterizerState = RasterizerState::GetResource(graphics, m_pass->GetRasterizerOptions(), objectRasterizerOptions);
	RasterizerState* pRasterizerState = rasterizerState.get();

	m_bindableContainer.AddBindable(std::move(rasterizerState));

	return pRasterizerState;
}