#include "Cube.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "Macros/ErrorMacros.h"

#include "Shader.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "InputLayout.h"
#include "ConstantBuffer.h"
#include "PrimitiveTechnology.h"
#include "Sampler.h"

#include "DynamicConstantBuffer.h"

#include <imgui.h>

Cube::Cube(Graphics& graphics, Pipeline& pipeline, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation)
	:
	SceneObject(position, rotation)
{
	struct Vertice
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoords;
	};

	float aspectRatioX = float(graphics.GetWidth()) / graphics.GetHeight();

	std::vector<InputLayout::Item> layoutElements = { {"POSITION", InputLayout::ItemType::Position3}, {"TEXCOORDS", InputLayout::ItemType::TexCoords} };

	std::vector<Vertice> vertices = {
		//front
		{{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f}},
		{{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{1.0f,  1.0f, 1.0f}, {1.0f, 0.0f}},

		//back
		{{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f}},
		{{1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
		{{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
		
		//left
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
		{{-1.0f, -1.0f, 1.0f}, {1.0f, 1.0f}},
		{{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f}},
		{{-1.0f,  1.0f, 1.0f}, {1.0f, 0.0f}},

		//right
		{{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f}},
		{{1.0f,  1.0f, 1.0f}, {1.0f, 0.0f}},
		{{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f}},

		//top
		{{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
		{{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f}},
		{{1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},

		//bottom
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
		{{1.0f,  -1.0f, -1.0f}, {1.0f, 0.0f}},
		{{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f}},
	};

	std::vector<unsigned int> indices = {
		//front
		0,1,3,
		0,3,2,

		//back
		4,5,7,
		4,7,6,

		//left
		8,9,11,
		8,11,10,

		//right
		12,13,15,
		12,15,14,

		//top
		16,17,19,
		16,19,18,

		//bottom
		20,21,23,
		20,23,22,
	};

	SetVertexBuffer(std::make_shared<VertexBuffer>(graphics, vertices.data(), vertices.size(), sizeof(vertices.at(0))));
	SetIndexBuffer(std::make_shared<IndexBuffer>(graphics, indices));
	SetTransformConstantBuffer(std::make_shared<TransformConstantBuffer>(graphics, this));

	AddBindable(std::make_shared<Shader>("PS_Phong", ShaderType::PixelShader));
	AddBindable(std::make_shared<Shader>("VS_Phong", ShaderType::VertexShader));
	AddBindable(std::make_shared<BlendState>());
	AddBindable(std::make_shared<RasterizerState>());
	AddBindable(std::make_shared<DepthStencilState>());
	AddBindable(std::make_shared<InputLayout>(layoutElements));
	AddBindable(std::make_shared<PrimitiveTechnology>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

	AddStaticBindable(pipeline, "lightBuffer");

	{
		DynamicConstantBuffer::ConstantBufferLayout layout;
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("texcoordsScale");

		DynamicConstantBuffer::ConstantBufferData bufferData(layout);
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("texcoordsScale") = 1.0f;

		std::shared_ptr<CachedConstantBuffer> constBuffer =  std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 2}});
		m_constantBuffer = constBuffer.get();

		AddBindable(constBuffer);
	}

	AddBindable(std::make_shared<Texture>(graphics, L"brickwall.jpg"));

	AddBindable(std::make_shared<StaticSampler>(D3D12_FILTER_MIN_MAG_MIP_POINT));
}

void Cube::DrawImguiWindow(Graphics& graphics, bool isLayerVisible)
{
	if (!isLayerVisible)
		return;

	if (ImGui::Begin("Cube"))
	{
		bool changed = false;

		auto checkChanged = [&changed](bool expressionReturn) mutable
			{
				changed = changed || expressionReturn;
			};

		ImGui::Text("Position");
		checkChanged(ImGui::SliderFloat("x##position", &m_position.x, -100.0f, 100.0f));
		checkChanged(ImGui::SliderFloat("y##position", &m_position.y, -100.0f, 100.0f));
		checkChanged(ImGui::SliderFloat("z##position", &m_position.z, -100.0f, 100.0f));

		ImGui::Text("Rotation");
		checkChanged(ImGui::SliderAngle("x##rotation", &m_rotation.x, -180.0f, 180.0f));
		checkChanged(ImGui::SliderAngle("y##rotation", &m_rotation.y, -180.0f, 180.0f));
		checkChanged(ImGui::SliderAngle("z##rotation", &m_rotation.z, -180, 180.0f));

		checkChanged(ImGui::SliderFloat("Texcoords Scale", m_constantBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("texcoordsScale"), 0.1f, 10.0f));

		if (changed)
			m_constantBuffer->Update(graphics);
	}

	ImGui::End();
}