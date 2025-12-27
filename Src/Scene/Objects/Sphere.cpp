#include "Sphere.h"
#include "Includes/CppIncludes.h"
#include <Geometry.h>

#include "Includes/BindablesInclude.h"

#include "Graphics/Data/DynamicVertex.h"

#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"
#include "Scene/RenderTechnique.h"

#include <imgui.h>

Sphere::Sphere(Graphics& graphics, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, float diameter, size_t tesselation)
	:
	m_initialized(false),
	m_diameter(diameter),
	m_tesselation(tesselation)
{
	SetName("Sphere");

	Mesh modelMesh;

	m_transform.SetTransformConstantBuffer(std::make_shared<TransformConstantBuffer>(graphics));


	RenderTechnique technique(RenderJob::JobType::GeometryPass);
	RenderGraphicsStep step;
	{
		DynamicVertex::DynamicVertexLayout vertexLayout;
		vertexLayout.AddElement<DynamicVertex::ElementType::Position>();
		vertexLayout.AddElement<DynamicVertex::ElementType::Normal>();
		vertexLayout.AddElement<DynamicVertex::ElementType::TextureCoords>();

		step.AddBindable(InputLayout::GetBindableResource(graphics, vertexLayout));

		step.AddBindable(Shader::GetBindableResource(graphics, L"PS_WhiteColor", ShaderType::PixelShader));
		std::vector<const char*> macros = {"NORMAL", "TEXCOORDS"};
		step.AddBindable(Shader::GetBindableResource(graphics, L"VS", ShaderType::VertexShader));
		step.AddBindable(BlendState::GetBindableResource(graphics));
		step.AddBindable(RasterizerState::GetBindableResource(graphics));
		step.AddBindable(DepthStencilState::GetBindableResource(graphics));
		step.AddBindable(PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));
		step.AddBindable(m_transform.GetTransformConstantBuffer());
	}
	technique.AddStep(std::move(step));
	modelMesh.AddTechnique(std::move(technique));
	AddMesh(modelMesh);
}

void Sphere::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	// upload mesh data to object
	UpdateMesh(graphics);
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
		UpdateMesh(graphics);
}

void Sphere::UpdateMesh(Graphics& graphics)
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

	RenderGraphicsStep& albedoStep = modelMesh.GetTechnique(RenderJob::JobType::GeometryPass).GetStep(0);

	if (!m_initialized)
	{
		albedoStep.SetVertexBuffer(VertexBuffer::GetBindableResource(graphics, "Sphere", vertices.data(), vertexLayout, vertices.size()));
		albedoStep.SetIndexBuffer(IndexBuffer::GetBindableResource(graphics, "Sphere", indices));

		m_initialized = true;
	}
	else
	{
		albedoStep.GetBindableContainter().GetVertexBuffer()->Update(graphics, vertices.data(), vertices.size(), sizeof(vertices.front()));
		albedoStep.GetBindableContainter().GetIndexBuffer()->Update(graphics, indices.data(), indices.size(), sizeof(indices.front()));
	}
}