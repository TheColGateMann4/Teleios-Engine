#include "Mesh.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Scene/Objects/Camera.h"
#include "Graphics/RenderGraph/RenderJob/StepRenderJob.h"

void Mesh::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	for (auto& technique : m_techniques)
		technique.Initialize(graphics, pipeline);

	SubmitJobs(graphics.GetRenderer());
}

void Mesh::Update(Graphics& graphics, Pipeline& pipeline)
{
	for(auto& technique : m_techniques)
		technique.Update(graphics, pipeline);
}

void Mesh::AddTechnique(RenderTechnique&& technique)
{
	m_techniques.push_back(std::move(technique));
}

RenderTechnique& Mesh::GetTechnique(RenderJob::JobType type)
{
	auto found = std::find_if(
		m_techniques.begin(),
		m_techniques.end(),
		[type](const RenderTechnique& renderTechnique)
		{
			return renderTechnique.GetType() == type;
		}
	);

	THROW_INTERNAL_ERROR_IF("Failed to find technique by name", found == m_techniques.end());

	return *found;
}

std::vector<RenderTechnique>& Mesh::GetTechniques()
{
	return m_techniques;
}

void Mesh::SubmitJobs(Renderer& renderer)
{
	for (auto& technique : m_techniques)
		for (auto& step : technique.GetSteps())
		{		
			renderer.SubmitJob(
				std::make_shared<StepRenderJob>(
						technique.GetType(),
						&step
				)
			);
		}
}