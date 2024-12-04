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
#include "DynamicVertex.h"

#include <imgui.h>

Cube::Cube(Graphics& graphics, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation)
{
	Mesh cubeMesh(position, rotation);

	{
		struct Vertice
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT2 texCoords;
		};

		DynamicVertex::DynamicVertexLayout vertexLayout;
		vertexLayout.AddElement<DynamicVertex::ElementType::Position>();
		vertexLayout.AddElement<DynamicVertex::ElementType::TextureCoords>();

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

		cubeMesh.SetVertexBuffer(std::make_shared<VertexBuffer>(graphics, vertices.data(), vertices.size(), sizeof(vertices.at(0))));
		cubeMesh.SetIndexBuffer(std::make_shared<IndexBuffer>(graphics, indices));
		cubeMesh.SetTransformConstantBuffer(std::make_shared<TransformConstantBuffer>(graphics));

		cubeMesh.AddBindable(std::make_shared<Shader>(graphics, L"PS_Phong", ShaderType::PixelShader));
		cubeMesh.AddBindable(std::make_shared<Shader>(graphics, L"VS_Phong", ShaderType::VertexShader));
		cubeMesh.AddBindable(std::make_shared<BlendState>(graphics));
		cubeMesh.AddBindable(std::make_shared<RasterizerState>(graphics));
		cubeMesh.AddBindable(std::make_shared<DepthStencilState>(graphics));
		cubeMesh.AddBindable(std::make_shared<InputLayout>(graphics, vertexLayout));
		cubeMesh.AddBindable(std::make_shared<PrimitiveTechnology>(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

		cubeMesh.AddStaticBindable("lightBuffer");

		{
			DynamicConstantBuffer::ConstantBufferLayout layout;
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("texcoordsScale");

			DynamicConstantBuffer::ConstantBufferData bufferData(layout);
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("texcoordsScale") = 1.0f;

			std::shared_ptr<CachedConstantBuffer> constBuffer = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 2}});
			m_constantBuffer = constBuffer.get();

			cubeMesh.AddBindable(constBuffer);
		}

		cubeMesh.AddBindable(std::make_shared<Texture>(graphics, "Images/brickwall.jpg"));

		cubeMesh.AddBindable(std::make_shared<StaticSampler>(graphics, D3D12_FILTER_MIN_MAG_MIP_POINT));
	}

	AddMesh(cubeMesh);
}