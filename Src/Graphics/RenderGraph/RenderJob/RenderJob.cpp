#include "RenderJob.h"
#include "Macros/ErrorMacros.h"

RenderJob::JobGroup RenderJob::GetJobGroup(JobType jobType)
{
	switch (jobType)
	{
		case JobType::None:
			return JobGroup::None;

		case JobType::GBuffer:
		case JobType::Depth:
		case JobType::Emissive:
			return JobGroup::Geometry;

		default:
			THROW_INTERNAL_ERROR("Failed to map job type to group");
	}
}

RenderJob::RenderJob(JobType type)
	:
	m_type(type)
{

}

void RenderJob::LinkToPass(GeometryPass* pass)
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