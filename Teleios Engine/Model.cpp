#include "Model.h"

#include "Macros/ErrorMacros.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

Model::Model(Graphics& graphics, const char* path, float scale)
{
	std::string filePath = path;
	std::string fileName;

	// setting file path and name
	{
		size_t lastSlashPosition = filePath.rfind('\\');

		if (lastSlashPosition == std::string::npos)
		{
			lastSlashPosition = filePath.rfind('/');
		}

		fileName = std::string(filePath.begin() + lastSlashPosition + 1, filePath.end());
		filePath = std::string(filePath.begin(), filePath.begin() + lastSlashPosition + 1);
	}

	std::string targetFile = "../../" + filePath + fileName;

 	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(targetFile.c_str(),
		aiProcess_ConvertToLeftHanded | 
		aiProcess_CalcTangentSpace |
		aiProcess_GenNormals |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	THROW_INTERNAL_ERROR_IF(importer.GetErrorString(), scene == nullptr);

	for (size_t meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
	{
		aiMesh* mesh = scene->mMeshes[meshIndex];
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		m_meshes.push_back(std::make_shared<ModelMesh>(graphics, mesh, material, filePath, scale));
		AddMesh(m_meshes.back().get());
	}
}