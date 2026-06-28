#include "GraphicsStepRenderJob.h"

#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"
#include "Graphics/RenderGraph/RenderPass/Geometry/GeometryPass.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/CommandList.h"

#include "Includes/BindablesInclude.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"

#include "Scene/Material.h"

#include "Graphics/Core/Pix.h"

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
	auto attributeVertexEntry = m_bindableContainer.GetAttributeVertexBufferEntry();
	auto positionVertexEntry = m_bindableContainer.GetPositionVertexBufferEntry();
	auto indexBuffer = m_bindableContainer.GetIndexBufferEntry();

	if (attributeVertexEntry)
		attributeVertexEntry->GetVertexBuffer()->BindToCopyPipelineIfNeeded(graphics, pipeline);
	if (positionVertexEntry)
		positionVertexEntry->GetVertexBuffer()->BindToCopyPipelineIfNeeded(graphics, pipeline);

	THROW_INTERNAL_ERROR_IF("None vertex buffer was bound", !attributeVertexEntry && !positionVertexEntry);
	THROW_INTERNAL_ERROR_IF("Index buffer hasn't been bound", !indexBuffer);

	indexBuffer->GetIndexBuffer()->BindToCopyPipelineIfNeeded(graphics, pipeline);

	for (auto texture : m_bindableContainer.GetTextures())
		texture->InitializeGraphicResources(graphics, pipeline);

	for (auto* cachedBuffer : m_bindableContainer.GetCachedBuffers())
		cachedBuffer->Update(graphics);

	auto* material = m_step->GetMaterial();

	if (material)
		material->InitializeGraphicResources(graphics, pipeline);
}

bool GraphicsStepRenderJob::IsValid(RenderPass* pass, Scene& scene) const
{
	if (!m_step->IsEnabled())
		return false;

	unsigned int sceneIndex = m_step->GetSceneObject()->GetSceneIndex();
	GeometryPass* geometryPass = static_cast<GeometryPass*>(pass);

	return scene.IsVisible(geometryPass->GetActiveCameraIndex(), sceneIndex);
}

void GraphicsStepRenderJob::Execute(Graphics& graphics, CommandList* commandList) const
{
	START_CPU_EVENT(PIX_COLOR(0, 127, 127), "DrawIndexed");

	commandList->SetPipelineState(graphics, m_pipelineState.get());

	commandList->SetGraphicsRootSignature(graphics, m_rootSignature.get());

	std::shared_ptr<IndexBufferEntry> indexBufferEntry = m_bindableContainer.GetIndexBufferEntry();
	std::shared_ptr<VertexBufferEntry> vertexBufferEntry;
	{
		auto attribBufferEntry = m_bindableContainer.GetAttributeVertexBufferEntry();
		auto positionBufferEntry = m_bindableContainer.GetPositionVertexBufferEntry();

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
		auto* material = m_step->GetMaterial();

		if (material)
			material->Bind(graphics, commandList);
	}

	{
		const auto& commandListBindables = m_bindableContainer.GetCommandListBindables();

		for (auto& pCommandListBindable : commandListBindables)
			pCommandListBindable->BindToCommandList(graphics, commandList);

		vertexBufferEntry->BindToCommandList(graphics, commandList);
		indexBufferEntry->BindToCommandList(graphics, commandList);
	}

	unsigned int indices = m_bindableContainer.GetIndexBufferEntry()->GetIndexCount();
	unsigned int baseVertexOffset = vertexBufferEntry->GetEntryInfo().offset;
	unsigned int startIndexOffset = indexBufferEntry->GetEntryInfo().offset;

	commandList->DrawIndexed(graphics, indices, baseVertexOffset, startIndexOffset);

	END_CPU_EVENT();
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