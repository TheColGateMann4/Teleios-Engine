#include "Mesh.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Scene/Objects/Camera.h"

void Mesh::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	for (auto& technique : m_techniques)
		technique.Initialize(graphics, pipeline);

	SubmitJobs(graphics.GetRenderGraph());
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

RenderTechnique& Mesh::GetTechnique(const std::string& name)
{
	auto found = std::find_if(
		m_techniques.begin(),
		m_techniques.end(),
		[name](const RenderTechnique& renderTechnique)
		{
			return renderTechnique.GetName() == name;
		}
	);

	THROW_INTERNAL_ERROR_IF("Failed to find technique by name", found == m_techniques.end());

	return *found;
}

std::vector<RenderTechnique>& Mesh::GetTechniques()
{
	return m_techniques;
}

void Mesh::SubmitJobs(RenderGraph& renderGraph)
{
	for (auto& technique : m_techniques)
		for (auto& step : technique.GetSteps())
		{		
			renderGraph.SubmitJob(
				RenderJob{
					technique.GetName(),
					&step
				}
			);
		}
}