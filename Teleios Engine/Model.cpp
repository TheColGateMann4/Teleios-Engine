#include "Model.h"

#include "Macros/ErrorMacros.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

Model::Model(Graphics& graphics, const char* path)
{
#ifdef _DEBUG
	std::string filePath = std::string("../../Models/") + path;
#elif
	std::string filePath = std::string("Models/") + path;
#endif

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filePath.c_str(),
		aiProcess_ConvertToLeftHanded | 
		//aiProcess_CalcTangentSpace |
		aiProcess_GenNormals |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	THROW_INTERNAL_ERROR_IF(importer.GetErrorString(), scene == nullptr);

	for (size_t meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
	{
		aiMesh* mesh = scene->mMeshes[meshIndex];
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		m_meshes.push_back(std::make_shared<ModelMesh>(graphics, mesh, material));
		AddMesh(m_meshes.back().get());
	}
}