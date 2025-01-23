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
	std::string fileExtension;

	// setting file path and name
	{
		size_t lastDotPosition = filePath.rfind('.');

		if (lastDotPosition == std::string::npos)
			THROW_INTERNAL_ERROR("There was no extension given in model file name");

		size_t lastSlashPosition = filePath.rfind('\\');

		if (lastSlashPosition == std::string::npos)
			lastSlashPosition = filePath.rfind('/');

		if (lastSlashPosition == std::string::npos)
			lastSlashPosition = 0;
		else
			lastSlashPosition++;

		fileExtension = std::string(filePath.begin() + lastDotPosition, filePath.end());
		fileName = std::string(filePath.begin() + lastSlashPosition, filePath.end());
		filePath = std::string(filePath.begin(), filePath.begin() + lastSlashPosition);
	}

	std::string targetFile = "../../" + filePath + fileName;

	if(strcmp(fileExtension.c_str(), ".obj") == 0 || strcmp(fileExtension.c_str(), ".gltf") == 0)
	{
		Assimp::Importer importer;

		const aiScene* modelScene = importer.ReadFile(targetFile.c_str(),
			aiProcess_ConvertToLeftHanded |
			aiProcess_Triangulate |
			aiProcess_SortByPType |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_GenUVCoords |
			aiProcess_OptimizeMeshes |
			aiProcess_ValidateDataStructure
		);

		THROW_INTERNAL_ERROR_IF(importer.GetErrorString(), modelScene == nullptr);

		PushModel(graphics, scene, scale, filePath, modelScene->mRootNode, modelScene->mMeshes, modelScene->mMaterials);
	}
	else if (strcmp(fileExtension.c_str(), ".fbx") == 0)
	{

	}
	else 
	{
		std::string errorStr = "Invalid file extension was passed, extension: \"";
		errorStr += fileExtension;
		errorStr += "\".";

		THROW_INTERNAL_ERROR(errorStr.c_str());
	}
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