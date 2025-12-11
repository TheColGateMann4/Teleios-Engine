#include "Sphere.h"
#include "includes/CppIncludes.h"
#include <Geometry.h>

#include "Shader.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "InputLayout.h"
#include "PrimitiveTechnology.h"

#include "DynamicVertex.h"

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

	{
		DynamicVertex::DynamicVertexLayout vertexLayout;
		vertexLayout.AddElement<DynamicVertex::ElementType::Position>();
		vertexLayout.AddElement<DynamicVertex::ElementType::Normal>();
		vertexLayout.AddElement<DynamicVertex::ElementType::TextureCoords>();

		modelMesh.AddBindable(InputLayout::GetBindableResource(graphics, vertexLayout));

		modelMesh.AddBindable(Shader::GetBindableResource(graphics, L"PS_WhiteColor", ShaderType::PixelShader));
		std::vector<const char*> macros = {"NORMAL", "TEXCOORDS"};
		modelMesh.AddBindable(Shader::GetBindableResource(graphics, L"VS", ShaderType::VertexShader));
		modelMesh.AddBindable(BlendState::GetBindableResource(graphics));
		modelMesh.AddBindable(RasterizerState::GetBindableResource(graphics));
		modelMesh.AddBindable(DepthStencilState::GetBindableResource(graphics));
		modelMesh.AddBindable(PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));
		modelMesh.AddBindable(m_transform.GetTransformConstantBuffer());
	}

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

	if (!m_initialized)
	{
		modelMesh.SetVertexBuffer(VertexBuffer::GetBindableResource(graphics, "Sphere", vertices.data(), vertexLayout, vertices.size()));
		modelMesh.SetIndexBuffer(IndexBuffer::GetBindableResource(graphics, "Sphere", indices));

		m_initialized = true;
	}
	else
	{
		modelMesh.GetBindableContainter().GetVertexBuffer()->Update(graphics, vertices.data(), vertices.size(), sizeof(vertices.front()));
		modelMesh.GetBindableContainter().GetIndexBuffer()->Update(graphics, indices.data(), indices.size(), sizeof(indices.front()));
	}
}