#pragma once
#include "GraphicsRenderJob.h"
#include "Graphics/RenderGraph/RenderJob/GraphicsRenderData.h"
#include "Graphics/Core/BindableContainer.h"

class RenderGraphicsGeometryStep;
class GeometryPass;
class Material;

class RenderGraphicsGeometryJob : public GraphicsRenderJob
{
public:
	RenderGraphicsGeometryJob(GraphicsRenderData renderData, GeometryPass* pass);

public:
	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

	virtual void Update(Graphics& graphics) override;

	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	virtual bool IsValid(RenderPass* pass, Scene& scene) const override;

	virtual void Execute(Graphics& graphics, CommandList* commandList) const override;

	RenderGraphicsGeometryStep* GetStep() const;

private:
	RasterizerState* BuildAndGetRasterizerState(Graphics& graphics, Material* material);

	void BuildRootSignature(Graphics& graphics, Material* material);

	void BuildPipelineState(Graphics& graphics, Material* material);

protected:
	RenderGraphicsGeometryStep* m_step;
	GeometryPass* m_pass;

	BindableContainerRevision m_stepLastRevision;
	BindableContainerRevision m_materialLastRevision;
	BindableContainerRevision m_passLastRevision;
};