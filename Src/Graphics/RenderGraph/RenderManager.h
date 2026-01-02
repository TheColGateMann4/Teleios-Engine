#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/RenderGraph/RenderJob/RenderJob.h"
#include "Graphics/RenderGraph/RenderPass/RenderPass.h"

class RenderManager
{
	using PassList = std::vector<RenderPass*>;
	using PassListByJobType = std::array<PassList, static_cast<int>(RenderJob::JobType::Count)>;

public:
	void AddJob(std::shared_ptr<RenderJob> renderJob);

	void BindJobsToPasses(std::vector<std::shared_ptr<RenderPass>>& renderPasses);

private:
	RenderManager::PassListByJobType GetWantedJobTypesByPasses(std::vector<std::shared_ptr<RenderPass>>& renderPasses) const;

	void AssignJobsToPasses(const RenderManager::PassListByJobType& wantedJobsToPasses);

private:
	std::vector<std::shared_ptr<RenderJob>> m_allJobs;
};