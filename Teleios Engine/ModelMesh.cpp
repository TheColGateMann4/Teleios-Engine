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

		if (hasTextureCoords)
			vertexLayout.AddElement<DynamicVertex::ElementType::TextureCoords>();

		if (hasNormals)
			vertexLayout.AddElement<DynamicVertex::ElementType::Normal>();

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

			if (hasTextureCoords)
				vertexBuffer.Back().GetPropety<DynamicVertex::ElementType::TextureCoords>() = *reinterpret_cast<DirectX::XMFLOAT2*>(&mesh->mTextureCoords[0][vertexIndex]);

			if (hasNormals)
				vertexBuffer.Back().GetPropety<DynamicVertex::ElementType::Normal>() = *reinterpret_cast<DirectX::XMFLOAT3*>(&mesh->mNormals[vertexIndex]);

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

	std::vector<const char*> shaderMacros = {};
	shaderMacros.push_back("OUTPUT_CAMAERAPOS"); // phong requirement
	shaderMacros.push_back("INPUT_NORMAL"); // model objects will always have normals since we will generate them with assimp if they do not

	// handle material
	{
		ProcessMaterialPropeties(material);

		MaterialPropeties materialPropeties = ProcessMaterialPropeties(material);

		if(materialPropeties.hasAnyMap)
		{
			AddBindable(std::make_shared<StaticSampler>(D3D12_FILTER_MIN_MAG_MIP_POINT));

			shaderMacros.push_back("TEXTURE_ANY");
			shaderMacros.push_back("INPUT_TEXCCORDS"); // since we are handling textures, we will need texcoords argument provided to our shaders
		}

		if (materialPropeties.hasDiffuseMap)
		{
			AddBindable(std::make_shared<Texture>(graphics, materialPropeties.diffuseMapPath.c_str(), std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 0}}));
			shaderMacros.push_back("TEXTURE_DIFFUSE");
		}

		if (materialPropeties.hasNormalMap)
		{
			AddBindable(std::make_shared<Texture>(graphics, materialPropeties.normalMapPath.c_str(), std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}}));
			shaderMacros.push_back("TEXTURE_NORMAL");

			shaderMacros.push_back("INPUT_TANGENT");
			shaderMacros.push_back("INPUT_BITANGENT");
		}

		if (materialPropeties.hasSpecularMap)
		{
			AddBindable(std::make_shared<Texture>(graphics, materialPropeties.specularMapPath.c_str(), std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 2}}));
			shaderMacros.push_back("TEXTURE_SPECULAR");
		}
	}

	SetTransformConstantBuffer(std::make_shared<TransformConstantBuffer>(graphics, this));

	AddBindable(std::make_shared<Shader>("PS_Phong", ShaderType::PixelShader, shaderMacros));
	AddBindable(std::make_shared<Shader>("VS", ShaderType::VertexShader, shaderMacros));
	AddBindable(std::make_shared<InputLayout>(vertexLayout));

	AddBindable(std::make_shared<BlendState>());
	AddBindable(std::make_shared<RasterizerState>());
	AddBindable(std::make_shared<DepthStencilState>());
	AddBindable(std::make_shared<PrimitiveTechnology>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

	AddStaticBindable("lightBuffer");
}

ModelMesh::MaterialPropeties ModelMesh::ProcessMaterialPropeties(aiMaterial* material)
{
	MaterialPropeties resultPropeties;

	for (size_t propetyIndex = 0; propetyIndex < material->mNumProperties; propetyIndex++)
	{
		aiMaterialProperty* materialPropety = material->mProperties[propetyIndex];

		switch (material->mProperties[propetyIndex]->mType)
		{
			case aiPTI_Integer:
			{
				PrintMaterialPropeties<int>(materialPropety);
				break;
			}
			case aiPTI_Float:
			{
				PrintMaterialPropeties<float>(materialPropety);
				break;
			}
			case aiPTI_Double:
			{
				PrintMaterialPropeties<double>(materialPropety);
				break;

			}
			case aiPTI_String:
			{
				std::string texturePath = std::string("Models/nanosuit/") + reinterpret_cast<aiString*>(materialPropety->mData)->C_Str();
				unsigned int semantic = materialPropety->mSemantic;

				if (semantic != aiTextureType_NONE)
					resultPropeties.hasAnyMap = true;

				switch (semantic)
				{
					case aiTextureType_DIFFUSE:
					{
						resultPropeties.hasDiffuseMap = true;
						resultPropeties.diffuseMapPath = texturePath;
						break;
					}
					case aiTextureType_NORMALS:
					{
						resultPropeties.hasNormalMap = true;
						resultPropeties.normalMapPath = texturePath;
						break;
					}
					case aiTextureType_SPECULAR:
					{
						resultPropeties.hasSpecularMap = true;
						resultPropeties.specularMapPath = texturePath;
						break;
					}
					default: // if we are not handling the map we will print it in console
					{
						std::cout << "index: " << materialPropety->mIndex << '\n';
						std::cout << "key: " << materialPropety->mKey.C_Str() << '\n';
						std::cout << "semantic: " << semantic << '\n';
						std::cout << "data: \"" << texturePath << "\"\n";
						break;
					}
				}

				break;
			}
		}

		std::cout << "\n\n\n";
	}

	std::cout << "\n\n\n\n\n\n\n\n";

	return resultPropeties;
}

void ModelMesh::GetMaterialPropeties(aiMaterialProperty* materialPropety, void*& baseData, unsigned int& dataLength, unsigned int& index, const char*& key)
{
	baseData = materialPropety->mData;
	dataLength = materialPropety->mDataLength;
	index = materialPropety->mIndex;
	key = materialPropety->mKey.C_Str();
}