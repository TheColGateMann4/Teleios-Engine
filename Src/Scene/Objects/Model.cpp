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

BoundingBox CalculateBoundingBox(DynamicVertex::DynamicVertex& vertexData)
{
	BoundingBox result = {};

	int numVertices = vertexData.GetNumVertices();
	DirectX::XMFLOAT3* verticeData = static_cast<DirectX::XMFLOAT3*>(vertexData.GetData());

	THROW_INTERNAL_ERROR_IF("Mesh had invalid number of vertices", numVertices <= 0);

	for (int i = 0; i < numVertices; i++)
	{
		result.min.x = std::min<float>(verticeData[i].x, result.min.x);
		result.min.y = std::min<float>(verticeData[i].y, result.min.y);
		result.min.z = std::min<float>(verticeData[i].z, result.min.z);

		result.max.x = std::max<float>(verticeData[i].x, result.max.x);
		result.max.y = std::max<float>(verticeData[i].y, result.max.y);
		result.max.z = std::max<float>(verticeData[i].z, result.max.z);
	}

	return result;
}

void HandleVertexData(Graphics& graphics, RenderGraphicsStep& step, aiMesh* mesh, float scale)
{
	size_t numVertices = mesh->mNumVertices + 1;

	bool hasPositions = mesh->HasPositions();
	bool hasNormals = mesh->HasNormals();
	bool hasTextureCoords = mesh->HasTextureCoords(0);
	bool hasTangentsAndBitangent = mesh->HasTangentsAndBitangents();
	bool hasVertexColors = mesh->HasVertexColors(0);

	THROW_INTERNAL_ERROR_IF("Model didn't have vertex positions", !hasPositions);

	DynamicVertex::DynamicVertexLayout vertexLayout;

	// initializing vertex layout
	{
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

			// position
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

		std::string vbName = std::string(mesh->mName.C_Str()) + "#AttributeBuffer";
		step.SetAttributeBufferEntry(VertexBufferEntry::GetResource(graphics, vbName, vertexBuffer));
	}

	// creating position only vertex buffer
	{
		DynamicVertex::DynamicVertexLayout positionOnlyVertexLayout;
		positionOnlyVertexLayout.AddElement<DynamicVertex::ElementType::Position>();

		DynamicVertex::DynamicVertex positionOnlyVertexBuffer(positionOnlyVertexLayout, numVertices);
		for (size_t vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
		{
			positionOnlyVertexBuffer.EmplaceBack();

			DirectX::XMFLOAT3* pPosition = reinterpret_cast<DirectX::XMFLOAT3*>(&mesh->mVertices[vertexIndex]);

			positionOnlyVertexBuffer.Back().GetPropety<DynamicVertex::ElementType::Position>() = { pPosition->x * scale, pPosition->y * scale,pPosition->z * scale };
		}

		step.SetBoundingBox(CalculateBoundingBox(positionOnlyVertexBuffer));

		std::string vbName = std::string(mesh->mName.C_Str()) + "#PositionBuffer";
		step.SetPositionBufferEntry(VertexBufferEntry::GetResource(graphics, vbName, positionOnlyVertexBuffer));

	}

	step.AddBindable(InputLayout::GetResource(graphics, vertexLayout));
}

void HandleIndiceData(Graphics& graphics, RenderGraphicsStep& step, aiMesh* mesh)
{
	size_t mNumIndices = mesh->mNumFaces * mesh->mFaces->mNumIndices;
	std::vector<unsigned int> indices(mNumIndices, '\0');

	for (size_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
		for (size_t indiceIndex = 0; indiceIndex < mesh->mFaces[faceIndex].mNumIndices; indiceIndex++)
			indices.at(faceIndex * 3 + indiceIndex) = mesh->mFaces[faceIndex].mIndices[indiceIndex];

	step.AddBindable(IndexBuffer::GetResource(graphics, mesh->mName.C_Str(), indices));
}

Model::Model(Graphics& graphics, Model* pParent, aiNode* node, std::vector<std::pair<aiMesh*, std::shared_ptr<Material>>> modelMeshes, float scale, DirectX::XMFLOAT3 position)
	:
	SceneObject(pParent)
{

	// getting transfrom from assimp node
	{
		DirectX::XMMATRIX nodeTransform = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&node->mTransformation)));
		m_transform.SetFromMatrix(nodeTransform, position, scale);
	}

	for(auto& modelMesh : modelMeshes)
	{
		aiMesh* mesh = modelMesh.first;
		std::shared_ptr<Material> material = modelMesh.second;
		const MaterialProperties::MaterialProperties& materialPropeties = material->GetProperties();

		Mesh objectMesh;

		RenderTechnique technique(RenderJob::JobType::GBuffer);
		RenderGraphicsStep step;

		{
			HandleVertexData(graphics, step, mesh, scale);

			HandleIndiceData(graphics, step, mesh);

			step.SetMaterial(material);

			step.AddBindable(PrimitiveTechnology::GetResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));
		}

		technique.AddStep(std::move(step));
		objectMesh.AddTechnique(std::move(technique));
		AddMesh(objectMesh);
	}
}