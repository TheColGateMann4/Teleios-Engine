#pragma once

class Graphics;
class Pipeline;
class CommandList;
class GeometryPass;

class RenderJob
{
public:
	enum class JobType
	{
		None = -1,
		GBuffer = 0,
		DepthPass,

		Count
	};

public:
	RenderJob(JobType m_type);

public:
	void LinkToPass(GeometryPass* pass);

	virtual void GatherBindables();

	virtual void Initialize(Graphics& graphics, Pipeline& pipeline);

	virtual void Execute(Graphics& graphics, CommandList* commandList) const = 0;

	JobType GetType() const;

private:
	JobType m_type;

protected:
	GeometryPass* m_pass = nullptr;
};