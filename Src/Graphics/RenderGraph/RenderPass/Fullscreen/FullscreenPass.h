#pragma once
#include "Includes/BindablesInclude.h"
#include "Graphics/RenderGraph/RenderJob/RenderJob.h"
#include "Graphics/RenderGraph/RenderJob/RenderGraphicsFullscreenJob.h"
#include "Graphics/RenderGraph/RenderPass/RenderPass.h"
#include "Graphics/RenderGraph/Steps/RenderGraphicsFullscreenStep.h"

class FullscreenPass : public RenderPass
{
public:
	FullscreenPass(Graphics& graphics);
	virtual ~FullscreenPass() = default;

	// overriden freerly by fullscreen passes
	virtual void Initialize(Graphics& graphics, Scene& scene) override;

	// do not override, it creates a job and initializes step
	virtual void InitializePassResources(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

	// free to override
	virtual void InitializeFullscreenResources(Graphics& graphics, Pipeline& pipeline, Scene& scene);

	virtual void Update(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

	void AddBindable(std::shared_ptr<Bindable> bind);

public: // Execution
	virtual void ExecutePass(Graphics& graphics, CommandList* commandList, Scene& scene) override;

protected:
	virtual void PreDraw(Graphics& graphics, CommandList* commandList);
	virtual void PostDraw(Graphics& graphics, CommandList* commandList);

public:
	virtual void DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline);

protected:
	std::shared_ptr<RenderGraphicsFullscreenJob> m_job;
	std::shared_ptr<RenderGraphicsFullscreenStep> m_step;

	std::vector<std::shared_ptr<Bindable>> m_bindables;

	std::shared_ptr<VertexBufferEntry> m_vertexBufferEntry;
	std::shared_ptr<IndexBufferEntry> m_indexBufferEntry;
};