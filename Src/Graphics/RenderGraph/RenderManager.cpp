#include "RenderManager.h"

void RenderManager::AddJob(RenderJob&& renderJob)
{
	m_allJobs.push_back(std::move(renderJob));
}

void RenderManager::BindJobsToPasses(std::vector<std::shared_ptr<RenderPass>>& renderPasses)
{
	const RenderManager::PassListByJobType wantedJobsByPasses = GetWantedJobTypesByPasses(renderPasses);

	AssignJobsToPasses(wantedJobsByPasses);
}

RenderManager::PassListByJobType RenderManager::GetWantedJobTypesByPasses(std::vector<std::shared_ptr<RenderPass>>& renderPasses) const
{
	RenderManager::PassListByJobType m_targetPasses = {};

	for (auto& pRenderPass : renderPasses)
	{
		RenderJob::JobType wantedJob = pRenderPass->GetWantedJob();

		if (wantedJob == RenderJob::JobType::None)
			continue;

		PassList& targetVec = m_targetPasses.at(static_cast<int>(wantedJob));

		targetVec.push_back(pRenderPass.get());
	}

	return m_targetPasses;
}

void RenderManager::AssignJobsToPasses(const RenderManager::PassListByJobType& wantedJobsToPasses)
{
	for (auto& job : m_allJobs)
	{
		RenderJob::JobType jobType = job.GetType();

		if (jobType == RenderJob::JobType::None)
			continue;

		const PassList& targetPassesVector = wantedJobsToPasses.at(static_cast<int>(jobType));

		for (auto& targetPass : targetPassesVector)
			targetPass->AssignJob(&job);
	}
}