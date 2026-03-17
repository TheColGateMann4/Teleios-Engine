#pragma once
#include "StepRenderJob.h"
#include "Graphics/Core/BindableContainer.h"
#include "Graphics/Core/PipelineState.h"
#include "Graphics/RenderGraph/RenderJob/GraphicsRenderData.h"
#include "Graphics/Bindables/MaterialBindings.h"

class RenderGraphicsStep;
class Material;

class GraphicsStepRenderJob : public StepRenderJob
{
public:
	GraphicsStepRenderJob(GraphicsRenderData renderData, GeometryPass* pass);

public:
	virtual void GatherBindables() override;

	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	virtual void Execute(Graphics& graphics, CommandList* commandList) const override;

	virtual JobGroup GetGroup() const override;

	RenderGraphicsStep* GetStep() const;

private:
	void InitializeMaterialBindings();

	RasterizerState* BuildAndGetRasterizerState(Graphics& graphics, Material* material);

private:
	RenderGraphicsStep* m_step;

	MeshBindableContainer m_bindableContainer;
	
	std::shared_ptr<GraphicsPipelineState> m_pipelineState;

	std::shared_ptr<MaterialBindings> m_materialBindings;
};