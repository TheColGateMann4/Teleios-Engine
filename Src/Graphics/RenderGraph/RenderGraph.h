#pragma once
#include "Includes/CppIncludes.h"
#include "RenderPass/RenderPass.h"
#include "RenderManager.h"

class Graphics;
class Pipeline;
class Scene;
class CommandList;
class GeometryPass;

class RenderGraph
{
public:
	void Initialize(Graphics& graphics);

	void GatherJobBindables();

	void InitializeJobs(Graphics& graphics, Pipeline& pipeline);

	void InitializePasses(Graphics& graphics, Pipeline& pipeline, Scene& scene);

	void UpdatePasses(Graphics& graphics, Pipeline& pipeline, Scene& scene);

	// submits jobs that passes created
	void SubmitPassesJobs();

	void AssignJobsToPasses();

	void Execute(Graphics& graphics, CommandList* commandList);

	RenderManager& GetRenderManager();
	
private:
	void AddRenderPass(std::shared_ptr<RenderPass> renderPass);

private:
	std::vector<std::shared_ptr<RenderPass>> m_renderPasses;
	std::vector<GeometryPass*> m_geometryPasses;
	RenderManager m_renderManager;
};