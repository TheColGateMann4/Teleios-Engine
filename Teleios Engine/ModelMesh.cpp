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

ModelMesh::ModelMesh(Graphics& graphics, aiMesh* mesh, aiMaterial* material, std::string filePath, float scale, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation)
	:
	Drawable(position, rotation)
{
	std::string fileName = mesh->mName.C_Str();
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
			{
				DirectX::XMFLOAT3* pPosition = reinterpret_cast<DirectX::XMFLOAT3*>(&mesh->mVertices[vertexIndex]);

				vertexBuffer.Back().GetPropety<DynamicVertex::ElementType::Position>() = { pPosition->x * scale, pPosition->y * scale,pPosition->z * scale };
			}

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

		SetVertexBuffer(VertexBuffer::GetBindableResource(fileName, graphics, vertexBuffer));
	}


	//indices
	{
		size_t mNumIndices = mesh->mNumFaces * mesh->mFaces->mNumIndices;
		std::vector<unsigned int> indices(mNumIndices, '\0');

		for (size_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
			for (size_t indiceIndex = 0; indiceIndex < mesh->mFaces[faceIndex].mNumIndices; indiceIndex++)
				indices.push_back(mesh->mFaces[faceIndex].mIndices[indiceIndex]);

		SetIndexBuffer(IndexBuffer::GetBindableResource(fileName, graphics, indices));
	}

	std::vector<const char*> shaderMacros = {};
	shaderMacros.push_back("OUTPUT_CAMAERAPOS"); // phong requirement
	shaderMacros.push_back("INPUT_NORMAL"); // model objects will always have normals since we will generate them with assimp if they do not

	// handle material
	{
		MaterialPropeties materialPropeties = ProcessMaterialPropeties(material);

		// textures
		if(materialPropeties.hasAnyMap)
		{
			AddBindable(StaticSampler::GetBindableResource(D3D12_FILTER_MIN_MAG_MIP_POINT));

			shaderMacros.push_back("TEXTURE_ANY");
			shaderMacros.push_back("INPUT_TEXCCORDS"); // since we are handling textures, we will need texcoords argument provided to our shaders


			if (materialPropeties.hasDiffuseMap)
			{
				AddBindable(Texture::GetBindableResource(graphics, (filePath + materialPropeties.diffuseMapPath).c_str(), std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 0}}));
				shaderMacros.push_back("TEXTURE_DIFFUSE");
			}

			if (materialPropeties.hasNormalMap)
			{
				AddBindable(Texture::GetBindableResource(graphics, (filePath + materialPropeties.normalMapPath).c_str(), std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}}));
				shaderMacros.push_back("TEXTURE_NORMAL");

				shaderMacros.push_back("INPUT_TANGENT");
				shaderMacros.push_back("INPUT_BITANGENT");
			}

			if (materialPropeties.hasSpecularMap)
			{
				std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + materialPropeties.specularMapPath).c_str(), std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 2}});
				DXGI_FORMAT specularTextureFormat = specularTexture->GetFormat();

				materialPropeties.specularOneChannelOnly = specularTextureFormat == DXGI_FORMAT_R8_UNORM;

				AddBindable(std::move(specularTexture));
				shaderMacros.push_back("TEXTURE_SPECULAR");
			}
		}

		//Constant buffer describing model material propeties
		{
			DynamicConstantBuffer::ConstantBufferLayout layout;
			layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("ambient");
			layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("defaultDiffuseColor");
			layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("defaultSpecularColor");

			layout.AddElement<DynamicConstantBuffer::ElementType::Bool>("ignoreDiffseAlpha");
			layout.AddElement<DynamicConstantBuffer::ElementType::Bool>("specularOneChannelOnly");

			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("specularShinnynes");
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("specularPower");


			DynamicConstantBuffer::ConstantBufferData bufferData(layout);
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("ambient") = materialPropeties.ambientColor;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("defaultDiffuseColor") = materialPropeties.diffuseColor;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("defaultSpecularColor") = materialPropeties.specularColor;

			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Bool>("ignoreDiffseAlpha") = materialPropeties.ignoreDiffseAlpha;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Bool>("specularOneChannelOnly") = materialPropeties.specularOneChannelOnly;

			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("specularShinnynes") = materialPropeties.specularShinnynes;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("specularPower") = materialPropeties.specularPower;


			AddBindable(std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}}));
		}
	}

	SetTransformConstantBuffer(std::make_shared<TransformConstantBuffer>(graphics, this));

	AddBindable(Shader::GetBindableResource("PS_Phong", ShaderType::PixelShader, shaderMacros));
	AddBindable(Shader::GetBindableResource("VS", ShaderType::VertexShader, shaderMacros));
	AddBindable(InputLayout::GetBindableResource(vertexLayout));

	AddBindable(BlendState::GetBindableResource());
	AddBindable(RasterizerState::GetBindableResource());
	AddBindable(DepthStencilState::GetBindableResource());
	AddBindable(PrimitiveTechnology::GetBindableResource(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

	AddStaticBindable("lightBuffer");
}

ModelMesh::MaterialPropeties ModelMesh::ProcessMaterialPropeties(aiMaterial* material)
{
	MaterialPropeties resultPropeties;

	aiString resultTexturePath = {};

	if(material->GetTexture(aiTextureType_DIFFUSE, 0, &resultTexturePath) == aiReturn_SUCCESS)
	{
		resultPropeties.hasAnyMap = true;

		resultPropeties.hasDiffuseMap = true;
		resultPropeties.diffuseMapPath = resultTexturePath.C_Str();
	}

	if(material->GetTexture(aiTextureType_NORMALS, 0, &resultTexturePath) == aiReturn_SUCCESS)
	{
		resultPropeties.hasAnyMap = true;

		resultPropeties.hasNormalMap = true;
		resultPropeties.normalMapPath = resultTexturePath.C_Str();
	}

	if(material->GetTexture(aiTextureType_SPECULAR, 0, &resultTexturePath) == aiReturn_SUCCESS)
	{
		resultPropeties.hasAnyMap = true;

		resultPropeties.hasSpecularMap = true;
		resultPropeties.specularMapPath = resultTexturePath.C_Str();
	}


	(void)material->Get(AI_MATKEY_COLOR_AMBIENT, resultPropeties.ambientColor); // we can ignore if the function succeded since we have this member initialized

	(void)material->Get(AI_MATKEY_COLOR_DIFFUSE, resultPropeties.diffuseColor);

	(void)material->Get(AI_MATKEY_COLOR_SPECULAR, resultPropeties.specularColor);


	(void)material->Get(AI_MATKEY_SHININESS, resultPropeties.specularPower);

	(void)material->Get(AI_MATKEY_SHININESS_STRENGTH, resultPropeties.specularShinnynes);

	return resultPropeties;
}