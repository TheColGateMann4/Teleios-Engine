#include "Mesh.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Scene/Objects/Camera.h"
#include "Graphics/RenderGraph/RenderJob/StepRenderJob.h"

#include "Graphics/Bindables/VertexBuffer.h"
#include "Graphics/Bindables/IndexBuffer.h"
#include "Graphics/Bindables/Texture.h"
#include "Graphics/Bindables/Shader.h"
#include "Graphics/Bindables/DepthStencilState.h"
#include "Graphics/Bindables/BlendState.h"
#include "Graphics/Bindables/PrimitiveTechnology.h"
#include "Graphics/Bindables/InputLayout.h"
#include "Graphics/Bindables/RasterizerState.h"

void Mesh::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	CreateImplicitTechniques(graphics, pipeline);

	for (auto& technique : m_techniques)
		technique.Initialize(graphics, pipeline);
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
	RenderTechnique* targetTechnique = m_GetTechnique(type);

	THROW_INTERNAL_ERROR_IF("Failed to find technique by name", targetTechnique == nullptr);

	return *targetTechnique;
		}

bool Mesh::HasTechnique(RenderJob::JobType type)
{
	RenderTechnique* targetTechnique = m_GetTechnique(type);

	return targetTechnique != nullptr;
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

void Mesh::CreateImplicitTechniques(Graphics& graphics, Pipeline& pipeline)
{
	if(HasTechnique(RenderJob::JobType::GeometryPass) && !HasTechnique(RenderJob::JobType::DepthPass))
		CreateDepthTechnique(graphics, pipeline);
}

void Mesh::CreateDepthTechnique(Graphics& graphics, Pipeline& pipeline)
{
	const RenderTechnique& geometryTechnique = GetTechnique(RenderJob::JobType::GeometryPass);

	const RenderGraphicsStep& geometryStep = geometryTechnique.GetStep(0);

	const MeshBindableContainer& geometryBindables = geometryStep.GetBindableContainter();

	bool hasOpacity = StepHasOpacity(geometryStep);

	//creating technique
	{
		RenderTechnique depthTechnique(RenderJob::JobType::DepthPass);

		{
			RenderGraphicsStep depthStep;		
			
			depthStep.AddBindable(geometryBindables.GetVertexBuffer());
			depthStep.AddBindable(geometryBindables.GetIndexBuffer());

			depthStep.AddBindable(Shader::GetBindableResource(graphics, L"VS", ShaderType::VertexShader));

			depthStep.AddBindable(geometryBindables.GetInputLayout());
			depthStep.AddBindable(pipeline.GetStaticResource("lightBuffer"));

			depthStep.AddBindable(geometryBindables.GetTransformConstantBuffer());

			depthStep.AddBindable(RasterizerState::GetBindableResource(graphics, false));
			depthStep.AddBindable(DepthStencilState::GetBindableResource(graphics, DepthStencilState::DepthComparisonFunc::Less));
			depthStep.AddBindable(BlendState::GetBindableResource(graphics));
			depthStep.AddBindable(PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

			depthTechnique.AddStep(std::move(depthStep));
		}

		AddTechnique(std::move(depthTechnique));
	}
}

RenderTechnique* Mesh::m_GetTechnique(RenderJob::JobType type)
{
	auto found = std::find_if(
		m_techniques.begin(),
		m_techniques.end(),
		[type](const RenderTechnique& renderTechnique)
		{
			return renderTechnique.GetType() == type;
		}
	);

	return found != m_techniques.end() ? found._Ptr : nullptr;
}