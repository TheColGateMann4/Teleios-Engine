#include "Model.h"

#include "Shader.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "InputLayout.h"
#include "ConstantBuffer.h"
#include "PrimitiveTechnology.h"
#include "Sampler.h"

#include "DynamicVertex.h"
#include "DynamicConstantBuffer.h"

#include "Macros/ErrorMacros.h"

#include "Mesh.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

Model::Model(Graphics& graphics, Model* pParent, aiNode* node, std::vector<std::pair<aiMesh*, aiMaterial*>> modelMeshes, std::string filePath, float scale, DirectX::XMFLOAT3 position)
	:
	SceneObject(pParent)
{

	std::string fileName = std::string(node->mName.data, node->mName.length);

	// getting transfrom from assimp node
	{
		{
			DirectX::XMMATRIX nodeMatrix = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&node->mTransformation)));
			DirectX::XMVECTOR vecPos, vecRotQuat, vecScale;

			bool retVal = DirectX::XMMatrixDecompose(&vecScale, &vecRotQuat, &vecPos, nodeMatrix);

			THROW_INTERNAL_ERROR_IF("Couldn't decompose node matrix", !retVal);

			// 'float scale' is used to scale vertices and positions of objects, used "externally"
			// 'float3 modelScale' is internal model scale which is needed to preserve scene look
			DirectX::XMFLOAT3 modelScale;

			DirectX::XMStoreFloat3(&position, vecPos);
			DirectX::XMStoreFloat3(&modelScale, vecScale);

			position.x *= scale;
			position.y *= scale;
			position.z *= scale;

			m_transform.SetPosition(position);
			m_transform.SetQuaternionRotation(vecRotQuat);
			m_transform.SetScale(modelScale);
		}
	}

	m_transform.SetTransformConstantBuffer(std::make_shared<TransformConstantBuffer>(graphics));

	for(auto& modelMesh : modelMeshes)
	{
		aiMesh* mesh = modelMesh.first;
		aiMaterial* material = modelMesh.second;

		size_t numVertices = mesh->mNumVertices + 1;

		bool hasPositions = mesh->HasPositions();
		bool hasNormals = mesh->HasNormals();
		bool hasTextureCoords = mesh->HasTextureCoords(0);
		bool hasTangentsAndBitangent = mesh->HasTangentsAndBitangents();
		bool hasVertexColors = mesh->HasVertexColors(0);

		Mesh objectMesh;

		objectMesh.AddBindable(m_transform.GetTransformConstantBuffer());

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

			objectMesh.SetVertexBuffer(VertexBuffer::GetBindableResource(graphics, mesh->mName.C_Str(), vertexBuffer));
		}


		//indices
		{
			size_t mNumIndices = mesh->mNumFaces * mesh->mFaces->mNumIndices;
			std::vector<unsigned int> indices(mNumIndices, '\0');

			for (size_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
				for (size_t indiceIndex = 0; indiceIndex < mesh->mFaces[faceIndex].mNumIndices; indiceIndex++)
					indices.push_back(mesh->mFaces[faceIndex].mIndices[indiceIndex]);

			objectMesh.SetIndexBuffer(IndexBuffer::GetBindableResource(graphics, mesh->mName.C_Str(), indices));
		}

		std::vector<const wchar_t*> shaderMacros = {};
		shaderMacros.push_back(L"OUTPUT_CAMAERAPOS"); // phong requirement
		shaderMacros.push_back(L"INPUT_NORMAL"); // model objects will always have normals since we will generate them with assimp if they do not

		// handle material
		{
			MaterialPropeties materialPropeties = ProcessMaterialPropeties(material);

			shaderMacros.push_back(L"METALNESS_PIPELINE");

			if(materialPropeties.roughnessMetalnessInOneTexture)
				shaderMacros.push_back(L"METALNESS_ROUGHNESS_ONE_TEXTURE");

			// textures
			if (materialPropeties.hasAnyMap)
			{
				objectMesh.AddBindable(StaticSampler::GetBindableResource(graphics, D3D12_FILTER_MIN_MAG_MIP_POINT));

				if(materialPropeties.ignoreDiffseAlpha)
					shaderMacros.push_back(L"IGNORE_DIFFUSE_ALPHA");

				shaderMacros.push_back(L"TEXTURE_ANY");
				shaderMacros.push_back(L"INPUT_TEXCCORDS"); // since we are handling textures, we will need texcoords argument provided to our shaders


				if (materialPropeties.hasDiffuseMap)
				{
					objectMesh.AddBindable(Texture::GetBindableResource(graphics, (filePath + materialPropeties.diffuseMapPath).c_str(), true, true, { {ShaderVisibilityGraphic::PixelShader, 0} }));
					shaderMacros.push_back(L"TEXTURE_DIFFUSE");
				}

				if (materialPropeties.hasNormalMap)
				{
					objectMesh.AddBindable(Texture::GetBindableResource(graphics, (filePath + materialPropeties.normalMapPath).c_str(), true, false, { {ShaderVisibilityGraphic::PixelShader, 1} }));
					shaderMacros.push_back(L"TEXTURE_NORMAL");

					shaderMacros.push_back(L"INPUT_TANGENT");
					shaderMacros.push_back(L"INPUT_BITANGENT");
				}
				
				if(materialPropeties.metalRoughnessSystem)
				{
					if (materialPropeties.hasMetalnessMap)
					{
						std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + materialPropeties.specularMetalnessMapPath).c_str(), true, false, { {ShaderVisibilityGraphic::PixelShader, 3} });

						objectMesh.AddBindable(std::move(specularTexture));
						shaderMacros.push_back(L"TEXTURE_METALNESS");
					}

					if (materialPropeties.hasRoughnessMap)
					{
						std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + materialPropeties.specularMetalnessMapPath).c_str(), true, false, { {ShaderVisibilityGraphic::PixelShader, 4} });

						objectMesh.AddBindable(std::move(specularTexture));
						shaderMacros.push_back(L"TEXTURE_ROUGHNESS");
					}

					// reflectivity 5
				}
				else
				{
					if (materialPropeties.hasSpecularMap)
					{
						std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + materialPropeties.specularMetalnessMapPath).c_str(), true, false, { {ShaderVisibilityGraphic::PixelShader, 2} });

						materialPropeties.specularOneChannelOnly = specularTexture->GetFormat() == DXGI_FORMAT_R8_UNORM;

						objectMesh.AddBindable(std::move(specularTexture));
						shaderMacros.push_back(L"TEXTURE_SPECULAR");
					}
				}

				if (materialPropeties.hasAmbientMap)
				{
					std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + materialPropeties.specularMetalnessMapPath).c_str(), true, false, { {ShaderVisibilityGraphic::PixelShader, 6} });

					objectMesh.AddBindable(std::move(specularTexture));
					shaderMacros.push_back(L"TEXTURE_SPECULAR");
				}

				// opacity 7

			}

			//Constant buffer describing model material propeties
			{
				DynamicConstantBuffer::ConstantBufferLayout layout;
				{
					layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("ambient");
					layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("diffuse");

					if (materialPropeties.metalRoughnessSystem)
					{
						layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("reflectivity");
						layout.AddElement<DynamicConstantBuffer::ElementType::Float>("metalness", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 1.0f });
						layout.AddElement<DynamicConstantBuffer::ElementType::Float>("roughness", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 1.0f });
					}
					else
					{
						layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("defaultSpecularColor");
						layout.AddElement<DynamicConstantBuffer::ElementType::Bool>("specularOneChannelOnly", DynamicConstantBuffer::ImguiData{ false });
						layout.AddElement<DynamicConstantBuffer::ElementType::Float>("specular", DynamicConstantBuffer::ImguiFloatData{ true, 0.001f, 150.0f });
						layout.AddElement<DynamicConstantBuffer::ElementType::Float>("glosiness", DynamicConstantBuffer::ImguiFloatData{ true, 0.001f, 150.0f });
					}
					layout.AddElement<DynamicConstantBuffer::ElementType::Float>("opacity", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 1.0f });
				}

				DynamicConstantBuffer::ConstantBufferData bufferData(layout);
				{
					*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("ambient") = materialPropeties.ambient;
					*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("diffuse") = materialPropeties.diffuse;

					if (materialPropeties.metalRoughnessSystem)
					{
						*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("reflectivity") = materialPropeties.reflective;
						*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("metalness") = materialPropeties.metalness;
						*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("roughness") = materialPropeties.roughness;
					}
					else
					{
						*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("defaultSpecularColor") = materialPropeties.specularColor;
						*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Bool>("specularOneChannelOnly") = materialPropeties.specularOneChannelOnly;
						*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("specular") = materialPropeties.specular;
						*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("glosiness") = materialPropeties.glosiness;
					}
					*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("opacity") = materialPropeties.opacity;
				}

				objectMesh.AddBindable(std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}}));
			}

			objectMesh.AddBindable(RasterizerState::GetBindableResource(graphics, materialPropeties.twoSided));
		}

		objectMesh.AddBindable(Shader::GetBindableResource(graphics, L"PS", ShaderType::PixelShader, shaderMacros));
		objectMesh.AddBindable(Shader::GetBindableResource(graphics, L"VS", ShaderType::VertexShader, shaderMacros));
		objectMesh.AddBindable(InputLayout::GetBindableResource(graphics, vertexLayout));

		objectMesh.AddBindable(BlendState::GetBindableResource(graphics));
		objectMesh.AddBindable(DepthStencilState::GetBindableResource(graphics));
		objectMesh.AddBindable(PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

		objectMesh.AddStaticBindable("lightBuffer");

		AddMesh(objectMesh);
	}
}

Model::MaterialPropeties Model::ProcessMaterialPropeties(aiMaterial* material)
{
	MaterialPropeties resultPropeties;

	aiString resultTexturePath = {};

	if (material->GetTexture(aiTextureType_DIFFUSE, 0, &resultTexturePath) == aiReturn_SUCCESS)
	{
		resultPropeties.hasAnyMap = true;

		resultPropeties.hasDiffuseMap = true;
		resultPropeties.diffuseMapPath = std::string(resultTexturePath.data, resultTexturePath.length);
	}

	if (material->GetTexture(aiTextureType_NORMALS, 0, &resultTexturePath) == aiReturn_SUCCESS)
	{
		resultPropeties.hasAnyMap = true;

		resultPropeties.hasNormalMap = true;
		resultPropeties.normalMapPath = std::string(resultTexturePath.data, resultTexturePath.length);
	}

	if (material->GetTexture(aiTextureType_SPECULAR, 0, &resultTexturePath) == aiReturn_SUCCESS)
	{
		if (resultPropeties.hasMetalnessMap || resultPropeties.hasRoughnessMap)
			THROW_INTERNAL_ERROR("Tried to mix two PBR systems");

		resultPropeties.hasAnyMap = true;

		resultPropeties.hasSpecularMap = true;
		resultPropeties.specularMetalnessMapPath = std::string(resultTexturePath.data, resultTexturePath.length);
	}

	if (material->GetTexture(aiTextureType_METALNESS, 0, &resultTexturePath) == aiReturn_SUCCESS)
	{
		if (resultPropeties.hasSpecularMap || resultPropeties.hasGlosinessMap)
			THROW_INTERNAL_ERROR("Tried to mix two PBR systems");

		resultPropeties.hasAnyMap = true;

		resultPropeties.hasMetalnessMap = true;
		resultPropeties.metalRoughnessSystem = true;
		resultPropeties.specularMetalnessMapPath = std::string(resultTexturePath.data, resultTexturePath.length);
	}

	if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &resultTexturePath) == aiReturn_SUCCESS)
	{
		if (resultPropeties.hasSpecularMap || resultPropeties.hasGlosinessMap)
			THROW_INTERNAL_ERROR("Tried to mix two PBR systems");

		if (resultPropeties.hasMetalnessMap && strcmp(resultPropeties.specularMetalnessMapPath.c_str(), resultTexturePath.C_Str()) == 0)
		{
			resultPropeties.roughnessMetalnessInOneTexture = true;
		}
		else
		{
			resultPropeties.hasAnyMap = true;
			resultPropeties.hasRoughnessMap = true;
			resultPropeties.metalRoughnessSystem = true;
			resultPropeties.glosinessRoughnessMapPath = std::string(resultTexturePath.data, resultTexturePath.length);
		}
	}

	if (material->GetTexture(aiTextureType_AMBIENT, 0, &resultTexturePath) == aiReturn_SUCCESS)
	{
		resultPropeties.hasAnyMap = true;

		resultPropeties.hasAmbientMap = true;
		resultPropeties.ambientMapPath = std::string(resultTexturePath.data, resultTexturePath.length);
	}

	if (material->GetTexture(aiTextureType_OPACITY, 0, &resultTexturePath) == aiReturn_SUCCESS)
	{
		resultPropeties.hasAnyMap = true;

		resultPropeties.hasAmbientMap = true;
		resultPropeties.ambientMapPath = std::string(resultTexturePath.data, resultTexturePath.length);
	}


	(void)material->Get(AI_MATKEY_COLOR_AMBIENT, resultPropeties.ambient); // we can ignore if the function succeded since we have this member initialized

	(void)material->Get(AI_MATKEY_COLOR_DIFFUSE, resultPropeties.diffuse);

	(void)material->Get(AI_MATKEY_COLOR_SPECULAR, resultPropeties.specularColor);

	(void)material->Get(AI_MATKEY_COLOR_REFLECTIVE, resultPropeties.reflective);


	if (!resultPropeties.metalRoughnessSystem)
	{
		if (material->Get(AI_MATKEY_SHININESS, resultPropeties.specular) != aiReturn_SUCCESS || resultPropeties.specular == 0.0f)
		{
			resultPropeties.specular = 1.0f;
		}

		if (material->Get(AI_MATKEY_SHININESS_STRENGTH, resultPropeties.glosiness) != aiReturn_SUCCESS || resultPropeties.glosiness == 0.0f)
		{
			resultPropeties.glosiness = 1.0f;
		}
	}

	(void)material->Get(AI_MATKEY_TWOSIDED, resultPropeties.twoSided);

	(void)material->Get(AI_MATKEY_OPACITY, resultPropeties.opacity);

	return resultPropeties;
}