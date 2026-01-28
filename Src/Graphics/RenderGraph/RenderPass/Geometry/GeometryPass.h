#pragma once
#include "Graphics/RenderGraph/RenderPass/RenderPass.h"

class GeometryPass : public RenderPass
{
public: // Handling for pass specific bindables
	void AddBindable(std::shared_ptr<Bindable> bindable);
	const std::vector<std::shared_ptr<Bindable>>& GetBindables() const;

public: // job handling
	// sort jobs so executing them on GPU is more effecient
	void SortJobs();

public:  // enlisting and pushing jobs
	// every renderPass that will inherit will return its own wanted jobs, like "Albedo"
	virtual RenderJob::JobType GetWantedJob() const;

	// called by RenderManager to assign jobs that RenderPass enlisted for
	void AssignJob(std::shared_ptr<RenderJob> pJob);

protected:
	virtual void ExecutePass(Graphics& graphics, CommandList* commandList) override;

private:
	std::vector<std::shared_ptr<Bindable>> m_bindables;

	std::vector<std::shared_ptr<RenderJob>> m_pJobs;
};