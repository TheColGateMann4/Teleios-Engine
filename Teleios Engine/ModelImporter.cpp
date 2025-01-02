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

	for (size_t meshIndex = 0; meshIndex < modelScene->mNumMeshes; meshIndex++)
	{
		aiMesh* mesh = modelScene->mMeshes[meshIndex];
		aiMaterial* material = modelScene->mMaterials[mesh->mMaterialIndex];

		scene.AddSceneObjectFromFile(std::make_shared<Model>(graphics, mesh, material, filePath, scale), mesh->mName.C_Str());
	}
}