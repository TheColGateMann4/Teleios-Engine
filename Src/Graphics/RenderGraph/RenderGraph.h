#pragma once
#include "Includes/CppIncludes.h"
#include "RenderPass/RenderPass.h"
#include "RenderManager.h"

class Graphics;
class CommandList;

class RenderGraph
{
public:
	void Initialize(Graphics& graphics);

	void InitializePassResources(Graphics& graphics, Pipeline& pipeline);

	void RebindJobs();

	void Execute(Graphics& graphics, CommandList* commandList);

	RenderManager& GetRenderManager();
	
private:
	void AddRenderPass(std::shared_ptr<RenderPass> renderPass);

private:
	std::vector<std::shared_ptr<RenderPass>> m_renderPasses;
	RenderManager m_renderManager;
};