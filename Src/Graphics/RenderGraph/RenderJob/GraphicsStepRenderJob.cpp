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

void GraphicsStepRenderJob::BuildRootSignature(Graphics& graphics, Material* material)
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

		for (auto& rootSignatureBindable : m_pass->GetBindableContainer().GetRootSignatureBindables())
			rootSignatureBindable->AddGraphicsRootSignatureParam(&rootParams);

		if(material)
			for (auto& rootSignatureBindable : material->GetBindableContainer().GetRootSignatureBindables())
				rootSignatureBindable->AddGraphicsRootSignatureParam(&rootParams);
	}

	m_rootSignatureLayout = rootParams.GetLayout();
	m_rootSignature = RootSignature::GetResource(graphics, std::move(rootParams));
}

void GraphicsStepRenderJob::BuildPipelineState(Graphics& graphics, Material* material)
{
	const std::vector<RenderPass::RenderTargetData>& renderTargets = m_pass->GetRenderTargets();
	RenderPass::DepthStencilData depthStencilView = m_pass->GetDepthStencilView();

	GraphicsPipelineStateParams pipelineStateParams = {};

	{
		for (auto& pPipelineStateBindable : m_step->GetBindableContainer().GetPipelineStateBindables())
			pPipelineStateBindable->AddPipelineStateParam(graphics, &pipelineStateParams);

		for (auto& pPipelineStateBindable : m_pass->GetBindableContainer().GetPipelineStateBindables())
			pPipelineStateBindable->AddPipelineStateParam(graphics, &pipelineStateParams);

		if (material)
			for (auto& pPipelineStateBindable : material->GetBindableContainer().GetPipelineStateBindables())
				pPipelineStateBindable->AddPipelineStateParam(graphics, &pipelineStateParams);

		pipelineStateParams.SetRasterizerState(BuildAndGetRasterizerState(graphics, material));

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

void GraphicsStepRenderJob::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	Material* material = m_step->GetMaterial();

	BuildRootSignature(graphics, material);

	BuildPipelineState(graphics, material);

	InitializeGraphicResources(graphics, pipeline);

	m_stepLastRevision = m_step->GetBindableContainer().GetRevision();

	if(material)
		m_materialLastRevision = material->GetBindableContainer().GetRevision();

	if(m_pass)
		m_passLastRevision = m_pass->GetBindableContainer().GetRevision();
}

void HandleRevision(auto* obj, BindableContainerRevision& cached, bool& rsDirty, bool& psoDirty)
{
	if (!obj)
		return;

	const auto& revision = obj->GetBindableContainer().GetRevision();

	if (revision.rootSignatureRevision != cached.rootSignatureRevision)
		rsDirty = true;

	if (revision.pipelineStateRevision != cached.pipelineStateRevision)
		psoDirty = true;

	cached = revision;
}

void GraphicsStepRenderJob::Update(Graphics& graphics)
{
	bool psoDirty = false;
	bool rsDirty = false;

	auto* stepMaterial = m_step->GetMaterial();

	HandleRevision(m_step, m_stepLastRevision, rsDirty, psoDirty);
	HandleRevision(stepMaterial, m_materialLastRevision, rsDirty, psoDirty);
	HandleRevision(m_pass, m_passLastRevision, rsDirty, psoDirty);

	if (rsDirty)
		BuildRootSignature(graphics, stepMaterial);

	if (psoDirty || rsDirty)
		BuildPipelineState(graphics, stepMaterial);
}

void GraphicsStepRenderJob::InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline)
{
	const auto& stepBindableContainer = m_step->GetBindableContainer();

	auto attributeVertexEntry = stepBindableContainer.GetAttributeVertexBufferEntry();
	auto positionVertexEntry = stepBindableContainer.GetPositionVertexBufferEntry();
	auto indexBuffer = stepBindableContainer.GetIndexBufferEntry();

	THROW_INTERNAL_ERROR_IF("None vertex buffer was bound", !attributeVertexEntry && !positionVertexEntry);
	THROW_INTERNAL_ERROR_IF("Index buffer hasn't been bound", !indexBuffer);
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

	const auto& stepBindableContainer = m_step->GetBindableContainer();
	const Material* material = m_step->GetMaterial();

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
		auto* material = m_step->GetMaterial();

		if (material)
			material->Bind(graphics, commandList);
	}

	{
		m_rootSignatureLayout.BindToCommandList(graphics, commandList);

		for (auto& pCommandListBindable : stepBindableContainer.GetCommandListBindables())
			pCommandListBindable->BindToCommandList(graphics, commandList);

		if(m_pass)
			for (auto& pCommandListBindable : m_pass->GetBindableContainer().GetCommandListBindables())
				pCommandListBindable->BindToCommandList(graphics, commandList);

		if(material)
			for (auto& pCommandListBindable : material->GetBindableContainer().GetCommandListBindables())
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

	m_step->AddBindable(std::move(rasterizerState));

	return pRasterizerState;
}