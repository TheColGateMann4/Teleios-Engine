#include "ModelImporter.h"

#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"

#include "Macros/ErrorMacros.h"

#include "Scene.h"
#include "Model.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

void ModelImporter::AddSceneObjectFromFile(Graphics& graphics, const char* path, float scale, Scene& scene)
{
	std::string filePath = path;
	std::string fileName;

	// setting file path and name
	{
		size_t lastSlashPosition = filePath.rfind('\\');

		if (lastSlashPosition == std::string::npos)
			lastSlashPosition = filePath.rfind('/');

		if (lastSlashPosition == std::string::npos)
			lastSlashPosition = 0;

		fileName = std::string(filePath.begin() + lastSlashPosition + 1, filePath.end());
		filePath = std::string(filePath.begin(), filePath.begin() + lastSlashPosition + 1);
	}

	std::string targetFile = "../../" + filePath + fileName;

	Assimp::Importer importer;

	const aiScene* modelScene = importer.ReadFile(targetFile.c_str(),
		aiProcess_ConvertToLeftHanded |
		aiProcess_CalcTangentSpace |
		aiProcess_GenNormals |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	THROW_INTERNAL_ERROR_IF(importer.GetErrorString(), modelScene == nullptr);

		PushModel(graphics, scene, scale, filePath, modelScene->mRootNode, modelScene->mMeshes, modelScene->mMaterials);
}

void ModelImporter::PushModel(Graphics& graphics, Scene& scene, float scale, std::string& filePath, aiNode* node, aiMesh** meshes, aiMaterial** materials, Model* pParent)
{
	std::vector<std::pair<aiMesh*, aiMaterial*>> modelMeshes;
	modelMeshes.reserve(node->mNumMeshes);

	for (unsigned int meshIndex = 0; meshIndex < node->mNumMeshes; meshIndex++)
	{
		aiMesh* targetMesh = meshes[node->mMeshes[meshIndex]];
		modelMeshes.push_back({ targetMesh, materials[targetMesh->mMaterialIndex] });
	}

	Model* pLocalModel;
	{
		std::shared_ptr<Model> pModel = std::make_shared<Model>(graphics, pParent, node, modelMeshes, filePath, scale);
		pLocalModel = pModel.get();

		scene.AddSceneObjectFromFile(std::move(pModel), std::string(node->mName.data, node->mName.length));
	}

	for (unsigned int childIndex = 0; childIndex < node->mNumChildren; childIndex++)
		PushModel(graphics, scene, scale, filePath, node->mChildren[childIndex], meshes, materials, pLocalModel);
}