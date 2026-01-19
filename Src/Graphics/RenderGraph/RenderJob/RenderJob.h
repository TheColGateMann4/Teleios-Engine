#pragma once

class Graphics;
class Pipeline;
class CommandList;
class RenderPass;

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
	void LinkToPass(RenderPass* pass);

	virtual void GatherBindables();

	virtual void Initialize(Graphics& graphics, Pipeline& pipeline);

	virtual void Execute(Graphics& graphics, CommandList* commandList) const = 0;

	JobType GetType() const;

private:
	JobType m_type;

protected:
	RenderPass* m_pass = nullptr;
};