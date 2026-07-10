#pragma once
#include "GraphicsRenderJob.h"
#include "Graphics/RenderGraph/RenderJob/GraphicsRenderData.h"

class RenderGraphicsFullscreenStep;
class FullscreenPass;

class RenderGraphicsFullscreenJob : public GraphicsRenderJob
{
public:
	RenderGraphicsFullscreenJob(GraphicsRenderData renderData, FullscreenPass* pass);

public:
	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	virtual void Execute(Graphics& graphics, CommandList* commandList) const override;

private:
	void BuildRootSignature(Graphics& graphics);

	void BuildPipelineState(Graphics& graphics);

private:
	RenderGraphicsFullscreenStep* m_step;
	FullscreenPass* m_pass;
};