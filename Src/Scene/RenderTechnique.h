#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"
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
	void Initialize(Graphics& graphics, Pipeline& pipeline);

	void Update(Graphics& graphics, Pipeline& pipeline);

public:
	void AddStep(RenderGraphicsStep&& step);

	RenderGraphicsStep& GetStep(unsigned int index = 0);

	std::vector<RenderGraphicsStep>& GetSteps();

private:
	std::vector<RenderGraphicsStep> m_steps;
	RenderJob::JobType m_type;
};