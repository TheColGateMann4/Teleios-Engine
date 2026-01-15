#pragma once

class Graphics;
class CommandList;

class RenderJob
{
public:
	enum class JobType
	{
		None = -1,
		GeometryPass = 0,
		DepthPass,

		FullscreenPass,

		Count
	};

public:
	RenderJob(JobType m_type);

public:
	virtual void Execute(Graphics& graphics, CommandList* commandList) const = 0;

	JobType GetType() const;

private:
	JobType m_type;
};