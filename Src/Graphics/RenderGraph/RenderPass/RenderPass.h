#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/Bindables/RenderTarget.h"
#include "Graphics/Bindables/DepthStencilView.h"
#include "Graphics/RenderGraph/RenderJob/RenderJob.h"

class Graphics;
class CommandList;
class RenderManager;

class RenderPass
{
public: 
	// resource initialization for passes with their own work
	virtual void Initialize(Graphics& graphics);
	virtual void InitializePassResources(Graphics& graphics, Pipeline& pipeline);

	virtual void SubmitJobs(RenderManager& renderManager);

	void SetCorrectStates(Graphics& graphics, CommandList* commandList);

	// record jobs on command list
	void Execute(Graphics& graphics, CommandList* commandList);

public: // RenderTargets and DepthStecilViews
	void AddRenderTarget(std::shared_ptr<RenderTarget> renderTarget);
	void SetDepthStencilView(std::shared_ptr<DepthStencilViewBase> depthStencil);

	const std::vector<std::shared_ptr<RenderTarget>>& GetRenderTargets() const;
	std::shared_ptr<DepthStencilViewBase> GetDepthStencilView() const;

protected:
	virtual void ExecutePass(Graphics& graphics, CommandList* commandList) = 0;

private:
	std::vector<std::shared_ptr<RenderTarget>> m_renderTargets;
	std::shared_ptr<DepthStencilViewBase> m_depthStencil;
};
