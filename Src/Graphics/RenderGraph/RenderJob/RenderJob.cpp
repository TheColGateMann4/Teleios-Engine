#include "RenderJob.h"

RenderJob::RenderJob(JobType type)
	:
	m_type(type)
{

}

RenderJob::JobType RenderJob::GetType() const
{
	return m_type;
}