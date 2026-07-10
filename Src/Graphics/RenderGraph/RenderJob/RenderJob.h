#pragma once

class RenderPass;
class Graphics;
class Pipeline;
class Scene;
class CommandList;
class GeometryPass;

class RenderJob
{
public:
	enum class JobType
	{
		None = -1,
		GBuffer = 0,
		Depth,
		Emissive,
		VisibleDebug,
		OccludedDebug,

		Count
	};
	enum class JobGroup
	{
		None = -1,
		Geometry,

		Count
	};

	static JobGroup GetJobGroup(JobType jobType);

public:
	RenderJob(JobType m_type);

public:
	virtual void GatherBindables();

	virtual void Initialize(Graphics& graphics, Pipeline& pipeline);

	virtual void Update(Graphics& graphics);

	virtual bool IsValid(RenderPass* pass, Scene& scene) const;

	virtual void Execute(Graphics& graphics, CommandList* commandList) const = 0;

	JobType GetType() const;

private:
	JobType m_type;

protected:
	bool m_initialized = false;
};