#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/RenderGraph/Steps/RenderGraphicsGeometryStep.h"
#include "Graphics/RenderGraph/RenderJob/RenderJob.h"

class RenderTechnique
{
public:
	RenderTechnique(RenderJob::JobType type);
	RenderTechnique(RenderTechnique&&) noexcept = default;
	RenderTechnique(const RenderTechnique&) = delete;

public:
	RenderJob::JobType GetType() const;

public:
	void AddStep(RenderGraphicsGeometryStep&& step);

	RenderGraphicsGeometryStep& GetStep(unsigned int index = 0);

	const RenderGraphicsGeometryStep& GetStep(unsigned int index = 0) const;

	std::vector<RenderGraphicsGeometryStep>& GetSteps();

private:
	std::vector<RenderGraphicsGeometryStep> m_steps;
	RenderJob::JobType m_type;
};