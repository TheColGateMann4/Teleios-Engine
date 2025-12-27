#pragma once
#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"

class RenderJob
{
public:
	enum class JobType
	{
		None = -1,
		GeometryPass = 0,

		Count
	};

public:
	RenderJob(JobType m_type, RenderStep* step);

public:
	void Execute(Graphics& graphics, CommandList* commandList) const;

	JobType GetType() const;

private:
	JobType m_type;
	RenderStep* m_step;
};