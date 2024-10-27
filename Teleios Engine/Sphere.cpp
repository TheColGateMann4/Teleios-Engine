#include "Sphere.h"
#include "includes/CppIncludes.h"
#include <Geometry.h>

#include "Shader.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "InputLayout.h"
#include "PrimitiveTechnology.h"

Sphere::Sphere(Graphics& graphics, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation)
	:
	SceneObject(position, rotation)
{
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
	std::vector<InputLayout::Item> layoutElements = { {"POSITION", InputLayout::ItemType::Position3}, {"NORMAL", InputLayout::ItemType::Normal}, {"TEXCOORDS", InputLayout::ItemType::TexCoords} };

	std::vector<DirectX::VertexPositionNormalTexture> vertices;
	std::vector<uint16_t> indices;

	DirectX::ComputeSphere(vertices, indices, 1.0f, 3, false, false);

	SetVertexBuffer(std::make_shared<VertexBuffer>(graphics, vertices.data(), vertices.size(), sizeof(vertices.at(0))));
	SetIndexBuffer(std::make_shared<IndexBuffer>(graphics, indices));
	AddBindable(std::make_shared<InputLayout>(layoutElements));
}