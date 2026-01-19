#pragma once
#include "StepRenderJob.h"
#include "Graphics/Core/BindableContainer.h"
#include "Graphics/Core/PipelineState.h"

class RenderGraphicsStep;

class GraphicsStepRenderJob : public StepRenderJob
{
public:
	GraphicsStepRenderJob(RenderJob::JobType m_type, RenderGraphicsStep* step);

public:
	virtual void GatherBindables() override;

	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	virtual void Execute(Graphics& graphics, CommandList* commandList) const override;

private:
	RenderGraphicsStep* m_step;

	MeshBindableContainer m_bindableContainer;
	
	std::unique_ptr<GraphicsPipelineState> m_pipelineState;
};