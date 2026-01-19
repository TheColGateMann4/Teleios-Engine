#pragma once
#include "RenderJob.h"
#include "Graphics/Core/RootSignature.h"

class Graphics;
class Pipeline;

class StepRenderJob : public RenderJob
{
public:
	StepRenderJob(RenderJob::JobType m_type);

	virtual void Execute(Graphics& graphics, CommandList* commandList) const = 0;

protected:
	std::shared_ptr<RootSignature> m_rootSignature;
};