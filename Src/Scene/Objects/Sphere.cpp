#include "Sphere.h"
#include "Includes/CppIncludes.h"
#include <Geometry.h>

#include "Includes/BindablesInclude.h"

#include "Graphics/Data/DynamicVertex.h"

#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"
#include "Scene/RenderTechnique.h"

#include <imgui.h>

Sphere::Sphere(Graphics& graphics, DirectX::XMFLOAT3 position, float diameter, size_t tesselation)
	:
	m_initialized(false),
	m_diameter(diameter),
	m_tesselation(tesselation)
{
	SetName("Sphere");
	m_transform.SetPosition(position);

	Mesh modelMesh;

	m_transform.SetTransformConstantBuffer(std::make_shared<TransformConstantBuffer>(graphics));


	RenderTechnique technique(RenderJob::JobType::Emissive);
	RenderGraphicsStep step;
	{
		DynamicVertex::DynamicVertexLayout vertexLayout;
		vertexLayout.AddElement<DynamicVertex::ElementType::Position>();
		vertexLayout.AddElement<DynamicVertex::ElementType::Normal>();
		vertexLayout.AddElement<DynamicVertex::ElementType::TextureCoords>();

		step.AddBindable(InputLayout::GetBindableResource(graphics, vertexLayout));

		step.AddBindable(Shader::GetBindableResource(graphics, L"PS_LightSource", ShaderType::PixelShader));
		std::vector<const char*> macros = {"NORMAL", "TEXCOORDS"};
		step.AddBindable(Shader::GetBindableResource(graphics, L"VS", ShaderType::VertexShader));
		step.AddBindable(BlendState::GetBindableResource(graphics));
		step.AddBindable(RasterizerState::GetBindableResource(graphics));
		step.AddBindable(DepthStencilState::GetBindableResource(graphics, DepthStencilState::DepthComparisonFunc::Less));
		step.AddBindable(PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));
		step.AddStaticBindable("lightBuffer");
		step.AddStaticBindable("cameraBuffer");
		step.AddBindable(m_transform.GetTransformConstantBuffer());

		// adding root signature constant to indicate locally used point light data
		{
			DynamicConstantBuffer::Layout layout;
			layout.Add<DynamicConstantBuffer::ElementType::Int>("pointLightIndex");

			layout.GetFinished(DynamicConstantBuffer::Layout::LayoutType::data);

			DynamicConstantBuffer::Data bufferData(layout);
			*bufferData.Get<DynamicConstantBuffer::ElementType::Int>("pointLightIndex") = 0;

			m_pointLightIndexConstant = std::make_shared<RootSignatureConstants>(bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}});

			step.AddBindable(m_pointLightIndexConstant);
		}
	}
	technique.AddStep(std::move(step));
	modelMesh.AddTechnique(std::move(technique));
	AddMesh(modelMesh);
}

void Sphere::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	// upload mesh data to object
	UpdateMesh(graphics, pipeline);
}

void Sphere::SetLightIndex(unsigned int lightIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to call SetLightIndex on sphere that wasn't meant as light source", !m_pointLightIndexConstant);

	*m_pointLightIndexConstant->GetData().Get<DynamicConstantBuffer::ElementType::Int>("pointLightIndex") = lightIndex;
}

void Sphere::DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline)
{
	bool changed = false;

	auto checkChanged = [&changed](bool expressionReturn) mutable
		{
			changed = changed || expressionReturn;
		};

	static constexpr int minTesselationVal = 3;
	static constexpr int maxTesselationVal = 64;


	ImGui::Text("Mesh Propeties");
	checkChanged(ImGui::SliderFloat("diameter", &m_diameter, 0.1f, 10.0f));
	checkChanged(ImGui::SliderScalar("tesselation", ImGuiDataType_U32, &m_tesselation, &minTesselationVal, &maxTesselationVal));

	if (changed)
		UpdateMesh(graphics, pipeline);
}

void Sphere::UpdateMesh(Graphics& graphics, Pipeline& pipeline)
{
	std::vector<DirectX::VertexPositionNormalTexture> vertices;
	std::vector<uint16_t> indices;

	DynamicVertex::DynamicVertexLayout vertexLayout;
	vertexLayout.AddElement<DynamicVertex::ElementType::Position>();
	vertexLayout.AddElement<DynamicVertex::ElementType::Normal>();
	vertexLayout.AddElement<DynamicVertex::ElementType::TextureCoords>();
	vertexLayout.Finish();

	DirectX::ComputeSphere(vertices, indices, m_diameter, m_tesselation, false, false);

	Mesh& modelMesh = m_meshes.front();

	RenderGraphicsStep& albedoStep = modelMesh.GetTechnique(RenderJob::JobType::Emissive).GetStep(0);

	if (!m_initialized)
	{
		albedoStep.AddBindable(VertexBuffer::GetBindableResource(graphics, "Sphere", vertices.data(), vertexLayout, vertices.size()));
		albedoStep.AddBindable(IndexBuffer::GetBindableResource(graphics, "Sphere", indices));

		m_initialized = true;
	}
	else
	{
		VertexBuffer* vbuffer = albedoStep.GetBindableContainter().GetVertexBuffer();
		IndexBuffer* ibuffer = albedoStep.GetBindableContainter().GetIndexBuffer();

		vbuffer->Update(graphics, vertices.data(), vertices.size(), sizeof(vertices.front()));
		ibuffer->Update(graphics, indices.data(), indices.size(), sizeof(indices.front()));
	
		vbuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);
		ibuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);
	}
}