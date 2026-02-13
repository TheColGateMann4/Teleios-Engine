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

Model::Model(Graphics& graphics, Model* pParent, aiNode* node, std::vector<std::pair<aiMesh*, std::shared_ptr<Material>>> modelMeshes, float scale, DirectX::XMFLOAT3 position)
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

			step.SetMaterial(material);

			step.AddBindable(InputLayout::GetBindableResource(graphics, vertexLayout));
			step.AddBindable(PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));
		}

		technique.AddStep(std::move(step));
		objectMesh.AddTechnique(std::move(technique));
		AddMesh(objectMesh);
	}
}