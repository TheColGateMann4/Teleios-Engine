#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"

class RenderTechnique
{
public:
	RenderTechnique(const std::string& name);
	RenderTechnique(RenderTechnique&&) noexcept = default;
	RenderTechnique(const RenderTechnique&) = delete;

public:
	const std::string& GetName() const;

public:
	void Initialize(Graphics& graphics, Pipeline& pipeline);

	void Update(Graphics& graphics, Pipeline& pipeline);

public:
	void AddStep(RenderGraphicsStep&& step);

	RenderGraphicsStep& GetStep(unsigned int index = 0);

	std::vector<RenderGraphicsStep>& GetSteps();

private:
	std::vector<RenderGraphicsStep> m_steps;
	std::string m_name;
};