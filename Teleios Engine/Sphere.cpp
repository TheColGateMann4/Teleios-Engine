#include "Sphere.h"
#include "includes/CppIncludes.h"
#include <Geometry.h>

#include "Shader.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "InputLayout.h"
#include "PrimitiveTechnology.h"

#include <imgui.h>

Sphere::Sphere(Graphics& graphics, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation)
	:
	SceneObject(position, rotation)
{
	std::vector<InputLayout::Item> layoutElements = { {"POSITION", InputLayout::ItemType::Position3}, {"NORMAL", InputLayout::ItemType::Normal}, {"TEXCOORDS", InputLayout::ItemType::TexCoords} };
	AddBindable(std::make_shared<InputLayout>(layoutElements));

	UpdateMesh(graphics);

	SetTransformConstantBuffer(std::make_shared<TransformConstantBuffer>(graphics, this));

	AddBindable(std::make_shared<Shader>("PS_WhiteColor", ShaderType::PixelShader));
	AddBindable(std::make_shared<Shader>("VS_Position_Normal_Texcoords", ShaderType::VertexShader));
	AddBindable(std::make_shared<BlendState>());
	AddBindable(std::make_shared<RasterizerState>());
	AddBindable(std::make_shared<DepthStencilState>());
	AddBindable(std::make_shared<PrimitiveTechnology>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));
}

void Sphere::UpdateMesh(Graphics& graphics)
{
	std::vector<DirectX::VertexPositionNormalTexture> vertices;
	std::vector<uint16_t> indices;

	DirectX::ComputeSphere(vertices, indices, m_diameter, m_tesselation, false, false);

	if(!m_initialized)
	{
		SetVertexBuffer(std::make_shared<VertexBuffer>(graphics, vertices.data(), vertices.size(), sizeof(vertices.at(0))));
		SetIndexBuffer(std::make_shared<IndexBuffer>(graphics, indices));

		m_initialized = true;
	}
	else
	{
		size_t previousIndexCount = m_indexBuffer->GetIndexCount();

		m_vertexBuffer->Update(graphics, vertices.data(), vertices.size(), sizeof(vertices.front()));
		m_indexBuffer->Update(graphics, indices.data(), indices.size(), sizeof(indices.front()));

		// if size was different in vertex and index buffers they were caused to resize. Which made new resource and previous resource views pushed to commandlist bundle are invalid
		// thats why we re-record the bundle list with our new resources bound to it
		if(previousIndexCount != m_indexBuffer->GetIndexCount())
			RecordBundleList(graphics);
	}
}

void Sphere::DrawImguiWindow(Graphics& graphics, bool isLayerVisible)
{
	if (!isLayerVisible)
		return;

	if (ImGui::Begin("Sphere"))
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

	ImGui::End();
}