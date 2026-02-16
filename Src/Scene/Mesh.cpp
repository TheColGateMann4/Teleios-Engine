#include "Mesh.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Scene/Objects/Camera.h"
#include "Graphics/RenderGraph/RenderJob/GraphicsStepRenderJob.h"

#include "Graphics/Bindables/VertexBuffer.h"
#include "Graphics/Bindables/IndexBuffer.h"
#include "Graphics/Bindables/Texture.h"
#include "Graphics/Bindables/Shader.h"
#include "Graphics/Bindables/DepthStencilState.h"
#include "Graphics/Bindables/BlendState.h"
#include "Graphics/Bindables/PrimitiveTechnology.h"
#include "Graphics/Bindables/InputLayout.h"
#include "Graphics/Bindables/RasterizerState.h"
#include "Graphics/Bindables/Sampler.h"

void Mesh::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	CreateImplicitTechniques(graphics, pipeline);
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
				std::make_shared<GraphicsStepRenderJob>(
						technique.GetType(),
						&step
				)
			);
		}
}

void Mesh::CreateImplicitTechniques(Graphics& graphics, Pipeline& pipeline)
{
	if(HasTechnique(RenderJob::JobType::GBuffer) && !HasTechnique(RenderJob::JobType::Depth))
		CreateDepthTechnique(graphics, pipeline);
}

void Mesh::CreateDepthTechnique(Graphics& graphics, Pipeline& pipeline)
{
	const RenderTechnique& geometryTechnique = GetTechnique(RenderJob::JobType::GBuffer);

	const RenderGraphicsStep& geometryStep = geometryTechnique.GetStep(0);

	const MeshBindableContainer& geometryBindables = geometryStep.GetBindableContainter();

	bool hasOpacity = StepHasOpacity(geometryStep);

	//creating technique
	{
		RenderTechnique depthTechnique(RenderJob::JobType::Depth);

		{
			RenderGraphicsStep depthStep;		
			
			depthStep.AddBindable(geometryBindables.GetVertexBuffer());
			depthStep.AddBindable(geometryBindables.GetIndexBuffer());

			depthStep.AddBindable(geometryBindables.GetInputLayout());

			depthStep.AddBindable(geometryBindables.GetTransformConstantBuffer());

			depthStep.AddBindable(RasterizerState::GetBindableResource(graphics, false));
			depthStep.AddBindable(DepthStencilState::GetBindableResource(graphics, DepthStencilState::DepthComparisonFunc::Less));
			depthStep.AddBindable(BlendState::GetBindableResource(graphics));
			depthStep.AddBindable(PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

			// if object has differing opacity then we need pixel shader to determine if we should clip individual pixels
			if (hasOpacity)
			{
				depthStep.AddBindable(Shader::GetBindableResource(graphics, L"PS_Depth", ShaderType::PixelShader));
				depthStep.AddBindable(Shader::GetBindableResource(graphics, L"VS", ShaderType::VertexShader, { { L"INPUT_TEXCCORDS" } }));

				// TODO: Handle for opacity texture
				depthStep.AddBindable(StaticSampler::GetBindableResource(graphics, D3D12_FILTER_MIN_MAG_MIP_POINT));
				depthStep.AddBindable(geometryStep.GetBindableContainter().GetTextures().front());
			}
			else
				depthStep.AddBindable(Shader::GetBindableResource(graphics, L"VS", ShaderType::VertexShader));

			depthTechnique.AddStep(std::move(depthStep));
		}

		AddTechnique(std::move(depthTechnique));
	}
}

bool Mesh::StepHasOpacity(const RenderGraphicsStep& geometryStep)
{
	const auto& geometryStepTextures = geometryStep.GetBindableContainter().GetTextures();

	if (geometryStepTextures.empty())
		return false;

	// TODO: DO a check for opacity texture

	// check diffuse texture alpha channel
	{
		// TODO: Make better system for accesing bindables of given step. For now this is correct since diffuse texture is always first
		// but edge case for now would be if given object doesn't have diffuse texture. Important to remember
		const Texture* diffuseTexture = geometryStepTextures.front();

		bool isAlphaOpaque = diffuseTexture->IsAlphaOpaque();

		return !isAlphaOpaque;
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