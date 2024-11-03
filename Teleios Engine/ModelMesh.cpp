#include "ModelMesh.h"

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

#include <assimp/scene.h>           // Output data structure

ModelMesh::ModelMesh(Graphics& graphics, aiMesh* mesh, aiMaterial* material, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation)
	:
	Drawable(position, rotation)
{
	size_t numVertices = mesh->mNumVertices + 1;

	bool hasPositions = mesh->HasPositions();
	bool hasNormals = mesh->HasNormals();
	bool hasTextureCoords = mesh->HasTextureCoords(0);
	bool hasTangentsAndBitangent = mesh->HasTangentsAndBitangents();
	bool hasVertexColors = mesh->HasVertexColors(0);

	DynamicVertex::DynamicVertexLayout vertexLayout;

	// initializing vertex layout
	{
		if (hasPositions)
			vertexLayout.AddElement<DynamicVertex::ElementType::Position>();

		if (hasNormals)
			vertexLayout.AddElement<DynamicVertex::ElementType::Normal>();

		if (hasTextureCoords)
			vertexLayout.AddElement<DynamicVertex::ElementType::TextureCoords>();

		if (hasTangentsAndBitangent)
		{
			vertexLayout.AddElement<DynamicVertex::ElementType::Tangent>();
			vertexLayout.AddElement<DynamicVertex::ElementType::Bitangent>();
		}

		if (hasVertexColors)
			vertexLayout.AddElement<DynamicVertex::ElementType::Color4>();
	}

	// pushing all the data to vertex buffer
	{
		DynamicVertex::DynamicVertex vertexBuffer(vertexLayout, numVertices);

		for (size_t vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
		{
			vertexBuffer.EmplaceBack();

			if (hasPositions)
				vertexBuffer.Back().GetPropety<DynamicVertex::ElementType::Position>() = *reinterpret_cast<DirectX::XMFLOAT3*>(&mesh->mVertices[vertexIndex]);

			if (hasNormals)
				vertexBuffer.Back().GetPropety<DynamicVertex::ElementType::Normal>() = *reinterpret_cast<DirectX::XMFLOAT3*>(&mesh->mNormals[vertexIndex]);

			if (hasTextureCoords)
				vertexBuffer.Back().GetPropety<DynamicVertex::ElementType::TextureCoords>() = *reinterpret_cast<DirectX::XMFLOAT2*>(&mesh->mTextureCoords[0][vertexIndex]);

			if (hasTangentsAndBitangent)
			{
				vertexBuffer.Back().GetPropety<DynamicVertex::ElementType::Tangent>() = *reinterpret_cast<DirectX::XMFLOAT3*>(&mesh->mTangents[vertexIndex]);
				vertexBuffer.Back().GetPropety<DynamicVertex::ElementType::Bitangent>() = *reinterpret_cast<DirectX::XMFLOAT3*>(&mesh->mBitangents[vertexIndex]);
			}

			if (hasVertexColors)
				vertexBuffer.Back().GetPropety<DynamicVertex::ElementType::Color4>() = *reinterpret_cast<DirectX::XMFLOAT4*>(&mesh->mColors[0][vertexIndex]);
		}

		SetVertexBuffer(std::make_shared<VertexBuffer>(graphics, vertexBuffer));
	}


	//indices
	{
		size_t mNumIndices = mesh->mNumFaces * mesh->mFaces->mNumIndices;
		std::vector<unsigned int> indices(mNumIndices, '\0');

		for (size_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
			for (size_t indiceIndex = 0; indiceIndex < mesh->mFaces[faceIndex].mNumIndices; indiceIndex++)
				indices.push_back(mesh->mFaces[faceIndex].mIndices[indiceIndex]);

		SetIndexBuffer(std::make_shared<IndexBuffer>(graphics, indices));
	}

	// handle material
	{
		//AddBindable(std::make_shared<StaticSampler>(D3D12_FILTER_MIN_MAG_MIP_POINT));

		// make some handler for material and make shader generator or something that would cooperate with it
	}

	SetTransformConstantBuffer(std::make_shared<TransformConstantBuffer>(graphics, this));

	AddBindable(std::make_shared<Shader>("PS_WhiteColor", ShaderType::PixelShader));
	AddBindable(std::make_shared<Shader>("VS", ShaderType::VertexShader));
	AddBindable(std::make_shared<InputLayout>(vertexLayout));

	AddBindable(std::make_shared<BlendState>());
	AddBindable(std::make_shared<RasterizerState>());
	AddBindable(std::make_shared<DepthStencilState>());
	AddBindable(std::make_shared<PrimitiveTechnology>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

	AddStaticBindable("lightBuffer");
}