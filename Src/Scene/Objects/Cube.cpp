#include "Cube.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Macros/ErrorMacros.h"

#include "Includes/BindablesInclude.h"

#include "Graphics/Data/DynamicConstantBuffer.h"
#include "Graphics/Data/DynamicVertex.h"

#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"
#include "Scene/RenderTechnique.h"

#include <imgui.h>

Cube::Cube(Graphics& graphics, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation)
{
	m_transform.SetTransformConstantBuffer(std::make_shared<TransformConstantBuffer>(graphics));

	Mesh cubeMesh;

	RenderTechnique technique(RenderJob::JobType::GBuffer);
	RenderGraphicsStep step;
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

		step.AddBindable(std::make_shared<VertexBuffer>(graphics, vertices.data(), vertices.size(), sizeof(vertices.at(0))));
		step.AddBindable(std::make_shared<IndexBuffer>(graphics, indices));
		step.AddBindable(m_transform.GetTransformConstantBuffer());

		step.AddBindable(std::make_shared<Shader>(graphics, L"PS_Phong", ShaderType::PixelShader));
		step.AddBindable(std::make_shared<Shader>(graphics, L"VS_Phong", ShaderType::VertexShader));
		step.AddBindable(std::make_shared<BlendState>(graphics));
		step.AddBindable(std::make_shared<RasterizerState>(graphics));
		step.AddBindable(std::make_shared<DepthStencilState>(graphics));
		step.AddBindable(std::make_shared<InputLayout>(graphics, vertexLayout));
		step.AddBindable(std::make_shared<PrimitiveTechnology>(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

		{
			DynamicConstantBuffer::Layout layout;
			layout.Add<DynamicConstantBuffer::ElementType::Float>("texcoordsScale");

			DynamicConstantBuffer::Data bufferData(layout);
			*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("texcoordsScale") = 1.0f;

			std::shared_ptr<CachedConstantBuffer> constBuffer = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 2}});
			m_constantBuffer = constBuffer.get();

			step.AddBindable(constBuffer);
		}

		step.AddBindable(std::make_shared<Texture>(graphics, "Images/brickwall.jpg"));

		step.AddBindable(std::make_shared<StaticSampler>(graphics, D3D12_FILTER_MIN_MAG_MIP_POINT));
	}
	technique.AddStep(std::move(step));
	cubeMesh.AddTechnique(std::move(technique));
	AddMesh(cubeMesh);
}