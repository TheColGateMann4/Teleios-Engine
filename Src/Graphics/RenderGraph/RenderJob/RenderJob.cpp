#include "RenderJob.h"

RenderJob::RenderJob(JobType type)
	:
	m_type(type)
{

}

void RenderJob::LinkToPass(RenderPass* pass)
{
	m_pass = pass;
}

void RenderJob::GatherBindables()
{

}

void RenderJob::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	
}

RenderJob::JobType RenderJob::GetType() const
{
	return m_type;
}