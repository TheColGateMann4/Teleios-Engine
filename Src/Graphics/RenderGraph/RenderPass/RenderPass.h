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
	// resource initialization for passes with their own jobs
	virtual void Initialize(Graphics& graphics);
	virtual void InitializePassResources(Graphics& graphics, Pipeline& pipeline);

	virtual void SubmitJobs(RenderManager& renderManager);

public: // Handling for pass specific bindables
	void AddBindable(std::shared_ptr<Bindable> bindable);
	const std::vector<std::shared_ptr<Bindable>>& GetBindables() const;


public: // RenderTargets and DepthStecilViews
	void AddRenderTarget(std::shared_ptr<RenderTarget> renderTarget);
	void SetDepthStencilView(std::shared_ptr<DepthStencilViewBase> depthStencil);

	const std::vector<std::shared_ptr<RenderTarget>>& GetRenderTargets() const;
	std::shared_ptr<DepthStencilViewBase> GetDepthStencilView() const;

public: // job handling
	// sort jobs so executing them on GPU is more effecient
	void SortJobs();

	// record jobs on command list
	void Execute(Graphics& graphics, CommandList* commandList);
	virtual void PreDraw(Graphics& graphics, CommandList* commandList);
	virtual void PostDraw(Graphics& graphics, CommandList* commandList);

public:  // enlisting and pushing jobs
	// every renderPass that will inherit will return its own wanted jobs, like "Albedo"
	virtual RenderJob::JobType GetWantedJob() const;

	// called by RenderManager to assign jobs that RenderPass enlisted for
	void AssignJob(std::shared_ptr<RenderJob> pJob);

private:
	std::vector<std::shared_ptr<Bindable>> m_bindables;

	std::vector<std::shared_ptr<RenderJob>> m_pJobs;

	std::vector<std::shared_ptr<RenderTarget>> m_renderTargets;
	std::shared_ptr<DepthStencilViewBase> m_depthStencil;
};
