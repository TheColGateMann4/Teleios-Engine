#include "RenderManager.h"

void RenderManager::AddJob(std::shared_ptr<RenderJob> renderJob)
{
	m_allJobs.push_back(std::move(renderJob));
}

void RenderManager::BindJobsToPasses(const std::vector<GeometryPass*>& renderPasses)
{
	const RenderManager::PassListByJobType wantedJobsByPasses = GetWantedJobTypesByPasses(renderPasses);

	AssignJobsToPasses(wantedJobsByPasses);
}

void RenderManager::GatherJobBindables()
{
	for(auto& job : m_allJobs)
		job->GatherBindables();
}

void RenderManager::InitializeJobs(Graphics& graphics, Pipeline& pipeline)
{
	for(auto& job : m_allJobs)
		job->Initialize(graphics, pipeline);
}

RenderManager::PassListByJobType RenderManager::GetWantedJobTypesByPasses(const std::vector<GeometryPass*>& renderPasses) const
{
	RenderManager::PassListByJobType m_targetPasses = {};

	for (const auto& pRenderPass : renderPasses)
	{
		RenderJob::JobType wantedJob = pRenderPass->GetWantedJob();

		if (wantedJob == RenderJob::JobType::None)
			continue;

		PassList& targetVec = m_targetPasses.at(static_cast<int>(wantedJob));

		targetVec.push_back(pRenderPass);
	}

	return m_targetPasses;
}

void RenderManager::AssignJobsToPasses(const RenderManager::PassListByJobType& wantedJobsToPasses)
{
	for (auto& job : m_allJobs)
	{
		RenderJob::JobType jobType = job->GetType();

		if (jobType == RenderJob::JobType::None)
			continue;

		const PassList& targetPassesVector = wantedJobsToPasses.at(static_cast<int>(jobType));

		for (auto& targetPass : targetPassesVector)
		{
			targetPass->AssignJob(job);
			job->LinkToPass(targetPass);
		}
	}
}