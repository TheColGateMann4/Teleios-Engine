#include "Model.h"

#include "Includes/BindablesInclude.h"

#include "Graphics/Resources/GraphicsTexture.h"

#include "Graphics/Data/DynamicVertex.h"
#include "Graphics/Data/DynamicConstantBuffer.h"

#include "Macros/ErrorMacros.h"

#include "Scene/Mesh.h"
#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"
#include "Scene/RenderTechnique.h"

#include "Scene/Material.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

Model::Model(Graphics& graphics, Model* pParent, aiNode* node, std::vector<std::pair<aiMesh*, std::shared_ptr<Material>>> modelMeshes, std::string filePath, float scale, DirectX::XMFLOAT3 position)
	:
	SceneObject(pParent)
{

	// getting transfrom from assimp node
	{
		DirectX::XMMATRIX nodeTransform = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&node->mTransformation)));
		m_transform.SetFromMatrix(nodeTransform, position, scale);
	}

	m_transform.SetTransformConstantBuffer(std::make_shared<TransformConstantBuffer>(graphics));

	for(auto& modelMesh : modelMeshes)
	{
		aiMesh* mesh = modelMesh.first;
		std::shared_ptr<Material> material = modelMesh.second;
		const MaterialProperties::MaterialProperties& materialPropeties = material->GetProperties();

		bool specularOneChannelOnly = false;

		size_t numVertices = mesh->mNumVertices + 1;

		bool hasPositions = mesh->HasPositions();
		bool hasNormals = mesh->HasNormals();
		bool hasTextureCoords = mesh->HasTextureCoords(0);
		bool hasTangentsAndBitangent = mesh->HasTangentsAndBitangents();
		bool hasVertexColors = mesh->HasVertexColors(0);

		Mesh objectMesh;

		RenderTechnique technique(RenderJob::JobType::GBuffer);
		RenderGraphicsStep step;

		{
			step.AddBindable(m_transform.GetTransformConstantBuffer());

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

				step.AddBindable(VertexBuffer::GetBindableResource(graphics, mesh->mName.C_Str(), vertexBuffer));
			}


			//indices
			{
				size_t mNumIndices = mesh->mNumFaces * mesh->mFaces->mNumIndices;
				std::vector<unsigned int> indices(mNumIndices, '\0');

				for (size_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
					for (size_t indiceIndex = 0; indiceIndex < mesh->mFaces[faceIndex].mNumIndices; indiceIndex++)
						indices.at(faceIndex * 3 + indiceIndex) = mesh->mFaces[faceIndex].mIndices[indiceIndex];

				step.AddBindable(IndexBuffer::GetBindableResource(graphics, mesh->mName.C_Str(), indices));
			}

			std::vector<ShaderMacro> shaderMacros = {};
			shaderMacros.push_back({ L"OUTPUT_CAMAERAPOS" }); // phong requirement
			shaderMacros.push_back({L"INPUT_NORMAL"}); // model objects will always have normals since we will generate them with assimp if they do not

			// handle material
			{
				shaderMacros.push_back({L"METALNESS_PIPELINE" });

				if (materialPropeties.roughnessMetalnessInOneTexture)
					shaderMacros.push_back({ L"METALNESS_ROUGHNESS_ONE_TEXTURE" });

				// textures
				if (materialPropeties.hasAnyMap)
				{
					step.AddBindable(StaticSampler::GetBindableResource(graphics, D3D12_FILTER_MIN_MAG_MIP_POINT));

					if (materialPropeties.ignoreDiffseAlpha)
						shaderMacros.push_back({ L"IGNORE_DIFFUSE_ALPHA" });

					shaderMacros.push_back({ L"TEXTURE_ANY" });
					shaderMacros.push_back({ L"INPUT_TEXCCORDS" }); // since we are handling textures, we will need texcoords argument provided to our shaders


					if (materialPropeties.hasAlbedoMap)
					{
						step.AddBindable(Texture::GetBindableResource(graphics, (filePath + materialPropeties.albedoMapPath).c_str(), true, true, true, { {ShaderVisibilityGraphic::PixelShader, 0} }));
						shaderMacros.push_back({ L"TEXTURE_DIFFUSE" });
					}

					if (materialPropeties.hasNormalMap)
					{
						step.AddBindable(Texture::GetBindableResource(graphics, (filePath + materialPropeties.normalMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 1} }));
						shaderMacros.push_back({ L"TEXTURE_NORMAL" });

						shaderMacros.push_back({ L"INPUT_TANGENT" });
						shaderMacros.push_back({ L"INPUT_BITANGENT" });
					}

					if (materialPropeties.metalRoughnessSystem)
					{
						if (materialPropeties.hasMetalnessMap)
						{
							std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + materialPropeties.specularMetalnessMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 3} });

							step.AddBindable(std::move(specularTexture));
							shaderMacros.push_back({ L"TEXTURE_METALNESS" });
						}

						if (materialPropeties.hasRoughnessMap)
						{
							std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + materialPropeties.specularMetalnessMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 4} });

							step.AddBindable(std::move(specularTexture));
							shaderMacros.push_back({ L"TEXTURE_ROUGHNESS" });
						}

						// reflectivity 5
					}
					else
					{
						if (materialPropeties.hasSpecularMap)
						{
							std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + materialPropeties.specularMetalnessMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 2} });

							specularOneChannelOnly = specularTexture->GetTexture()->GetFormat() == DXGI_FORMAT_R8_UNORM;

							step.AddBindable(std::move(specularTexture));
							shaderMacros.push_back({ L"TEXTURE_SPECULAR" });
						}
					}

					if (materialPropeties.hasAmbientMap)
					{
						std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + materialPropeties.ambientMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 6} });

						step.AddBindable(std::move(specularTexture));
						shaderMacros.push_back({ L"TEXTURE_AMBIENT" });
					}

					// opacity 7

				}

				//Constant buffer describing model material propeties
				{
					DynamicConstantBuffer::Layout layout;
					{
						layout.Add<DynamicConstantBuffer::ElementType::Float3>("ambient");
						layout.Add<DynamicConstantBuffer::ElementType::Float3>("diffuse");

						if (materialPropeties.metalRoughnessSystem)
						{
							layout.Add<DynamicConstantBuffer::ElementType::Float3>("reflectivity");
							layout.Add<DynamicConstantBuffer::ElementType::Float>("metalness", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 1.0f });
							layout.Add<DynamicConstantBuffer::ElementType::Float>("roughness", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 1.0f });
						}
						else
						{
							layout.Add<DynamicConstantBuffer::ElementType::Float3>("defaultSpecularColor");
							layout.Add<DynamicConstantBuffer::ElementType::Bool>("specularOneChannelOnly", DynamicConstantBuffer::ImguiData{ false });
							layout.Add<DynamicConstantBuffer::ElementType::Float>("specular", DynamicConstantBuffer::ImguiFloatData{ true, 0.001f, 150.0f });
							layout.Add<DynamicConstantBuffer::ElementType::Float>("glosiness", DynamicConstantBuffer::ImguiFloatData{ true, 0.001f, 150.0f });
						}
						layout.Add<DynamicConstantBuffer::ElementType::Float>("opacity", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 1.0f });
					}

					DynamicConstantBuffer::Data bufferData(layout);
					{
						*bufferData.Get<DynamicConstantBuffer::ElementType::Float3>("ambient") = materialPropeties.ambient;
						*bufferData.Get<DynamicConstantBuffer::ElementType::Float3>("diffuse") = materialPropeties.albedo;

						if (materialPropeties.metalRoughnessSystem)
						{
							*bufferData.Get<DynamicConstantBuffer::ElementType::Float3>("reflectivity") = materialPropeties.reflective;
							*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("metalness") = materialPropeties.metalness;
							*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("roughness") = materialPropeties.roughness;
						}
						else
						{
							*bufferData.Get<DynamicConstantBuffer::ElementType::Float3>("defaultSpecularColor") = materialPropeties.specularColor;
							*bufferData.Get<DynamicConstantBuffer::ElementType::Bool>("specularOneChannelOnly") = specularOneChannelOnly;
							*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("specular") = materialPropeties.specular;
							*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("glosiness") = materialPropeties.glosiness;
						}
						*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("opacity") = materialPropeties.opacity;
					}

					step.AddBindable(std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}}));
				}

				step.AddBindable(RasterizerState::GetBindableResource(graphics, materialPropeties.twoSided));
			}

			step.AddBindable(Shader::GetBindableResource(graphics, L"PS_GBuffer", ShaderType::PixelShader, shaderMacros));
			step.AddBindable(Shader::GetBindableResource(graphics, L"VS", ShaderType::VertexShader, shaderMacros));
			step.AddBindable(InputLayout::GetBindableResource(graphics, vertexLayout));

			step.AddBindable(BlendState::GetBindableResource(graphics, false));
			step.AddBindable(DepthStencilState::GetBindableResource(graphics, DepthStencilState::DepthComparisonFunc::Equal));
			step.AddBindable(PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

			step.AddStaticBindable("lightBuffer");
			step.AddStaticBindable("cameraBuffer");
		}

		technique.AddStep(std::move(step));
		objectMesh.AddTechnique(std::move(technique));
		AddMesh(objectMesh);
	}
}