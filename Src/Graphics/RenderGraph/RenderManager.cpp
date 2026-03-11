#include "RenderManager.h"

void RenderManager::AddRenderData(GraphicsRenderData renderData)
{
	m_allRenderData.push_back(renderData);
}

void RenderManager::BindJobsToPasses(const std::vector<GeometryPass*>& renderPasses)
{
	const RenderManager::PassListByJobType wantedDataByPasses = GetWantedDataTypesByPasses(renderPasses);

	AssignRenderDataToPasses(wantedDataByPasses);
}

RenderManager::PassListByJobType RenderManager::GetWantedDataTypesByPasses(const std::vector<GeometryPass*>& renderPasses) const
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

void RenderManager::AssignRenderDataToPasses(const RenderManager::PassListByJobType& wantedJobsToPasses)
{
	for (auto& renderData : m_allRenderData)
	{
		if (renderData.type == RenderJob::JobType::None)
			continue;

		const PassList& targetPassesVector = wantedJobsToPasses.at(static_cast<int>(renderData.type));

		for (auto& targetPass : targetPassesVector)
			targetPass->AssignRenderData(renderData);
	}
}