#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/RenderGraph/RenderJob/RenderJob.h"
#include "Graphics/RenderGraph/RenderPass/Geometry/GeometryPass.h"

class RenderManager
{
	using PassList = std::vector<GeometryPass*>;
	using PassListByJobType = std::array<PassList, static_cast<int>(RenderJob::JobType::Count)>;

public:
	void AddJob(std::shared_ptr<RenderJob> renderJob);

	void BindJobsToPasses(const std::vector<GeometryPass*>& renderPasses);

	void GatherJobBindables();

	void InitializeJobs(Graphics& graphics, Pipeline& pipeline);

private:
	RenderManager::PassListByJobType GetWantedJobTypesByPasses(const std::vector<GeometryPass*>& renderPasses) const;

	void AssignJobsToPasses(const RenderManager::PassListByJobType& wantedJobsToPasses);

private:
	std::vector<std::shared_ptr<RenderJob>> m_allJobs;
};