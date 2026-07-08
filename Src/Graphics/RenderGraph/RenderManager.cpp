#include "RenderManager.h"

void RenderManager::AddRenderData(GraphicsRenderData renderData)
{
	m_newRenderData.push_back(renderData);
}

void RenderManager::AssignNewJobsToPasses(const std::vector<GeometryPass*>& renderPasses)
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
	for (auto& renderData : m_newRenderData)
	{
		if (renderData.type == RenderJob::JobType::None)
			continue;

		const PassList& targetPassesVector = wantedJobsToPasses.at(static_cast<int>(renderData.type));

		for (auto& targetPass : targetPassesVector)
			targetPass->AssignRenderData(renderData);
	}

	// setting this data as assigned
	{
		m_submitedRenderData.reserve(m_submitedRenderData.size() + m_newRenderData.size());

		std::ranges::move(m_newRenderData, std::back_inserter(m_submitedRenderData));

		m_newRenderData.clear();
	}
}