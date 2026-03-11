#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/RenderGraph/RenderJob/RenderJob.h"
#include "Graphics/RenderGraph/RenderPass/Geometry/GeometryPass.h"
#include "Graphics/RenderGraph/RenderJob/GraphicsRenderData.h"

class RenderManager
{
	using PassList = std::vector<GeometryPass*>;
	using PassListByJobType = std::array<PassList, static_cast<int>(RenderJob::JobType::Count)>;

public:
	void AddRenderData(GraphicsRenderData renderData);

	void BindJobsToPasses(const std::vector<GeometryPass*>& renderPasses);

private:
	RenderManager::PassListByJobType GetWantedDataTypesByPasses(const std::vector<GeometryPass*>& renderPasses) const;

	void AssignRenderDataToPasses(const RenderManager::PassListByJobType& wantedJobsToPasses);

private:
	std::vector<GraphicsRenderData> m_allRenderData;
};