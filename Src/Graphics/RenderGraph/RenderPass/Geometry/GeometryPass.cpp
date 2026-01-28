#include "GeometryPass.h"

void GeometryPass::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindables.push_back(bindable);
}

const std::vector<std::shared_ptr<Bindable>>& GeometryPass::GetBindables() const
{
	return m_bindables;
}

void GeometryPass::SortJobs()
{
	// TODO: sort jobs by their PSO
}

RenderJob::JobType GeometryPass::GetWantedJob() const
{
	return RenderJob::JobType::None;
}

void GeometryPass::AssignJob(std::shared_ptr<RenderJob> pJob)
{
	m_pJobs.push_back(pJob);
}

void GeometryPass::ExecutePass(Graphics& graphics, CommandList* commandList)
{
	for (auto pJob : m_pJobs)
		pJob->Execute(graphics, commandList);
}