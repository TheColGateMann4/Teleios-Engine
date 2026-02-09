#include "ModelImporter.h"

#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"

#include "Macros/ErrorMacros.h"

#include "Scene.h"
#include "Objects/Model.h"
#include "Objects/PointLight.h"
#include "Objects/Camera.h"

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

#ifdef _DEBUG
	std::string targetFile = "../../" + filePath + fileName;
#else
	std::string targetFile = filePath + fileName;
#endif

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

		ProcessCameras(graphics, scene, *modelScene);
		ProcessLights(graphics, scene, *modelScene);
		ProcessMaterials(graphics, scene, *modelScene, filePath);

		PushModel(graphics, scene, scale, modelScene->mRootNode, modelScene->mMeshes, modelScene->mMaterials);
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

void ModelImporter::ProcessLights(Graphics& graphics, Scene& scene, const aiScene& importedScene)
{
	if (!importedScene.HasLights())
		return;

	std::span<aiLight*> importedLights(importedScene.mLights, importedScene.mNumLights);

	for (auto* importedLight : importedLights)
	{
		if(importedLight->mType == aiLightSource_POINT)
		{
			DirectX::XMFLOAT3 position = { importedLight->mPosition.x, importedLight->mPosition.y, importedLight->mPosition.z };
			DirectX::XMFLOAT3 color = { importedLight->mColorDiffuse.r, importedLight->mColorDiffuse.g, importedLight->mColorDiffuse.b };

			// mAttenuationConstant
			// mAttenuationLinear
			// mAttenuationQuadratic

			scene.AddSceneObject(std::make_shared<PointLight>(graphics, scene, position, color));
		}

		// mDirection
		// mAngleInnerCone
		// mAngleOuterCone
	}
}

void ModelImporter::ProcessCameras(Graphics& graphics, Scene& scene, const aiScene& importedScene)
{
	if (!importedScene.HasCameras())
		return;

	std::span<aiCamera*> importedCameras(importedScene.mCameras, importedScene.mNumCameras);

	for (auto* importedCamera : importedCameras)
	{
		DirectX::XMFLOAT3 position = { importedCamera->mPosition.x, importedCamera->mPosition.y, importedCamera->mPosition.z };
		DirectX::XMFLOAT3 rotation = { importedCamera->mLookAt.x, importedCamera->mLookAt.y, importedCamera->mLookAt.z };

		Camera::Settings cameraSettings = {};
		cameraSettings.FovAngleY = importedCamera->mHorizontalFOV;
		cameraSettings.AspectRatio = importedCamera->mAspect;
		cameraSettings.NearZ = importedCamera->mClipPlaneNear;
		cameraSettings.FarZ = importedCamera->mClipPlaneFar;

		scene.AddSceneObject(std::make_shared<Camera>(graphics, position, rotation, &cameraSettings));
	}
}

void ModelImporter::ProcessMaterials(Graphics& graphics, Scene& scene, const aiScene& importedScene, const std::string& filePath)
{
	if (!importedScene.HasMaterials())
		return;

	std::span<aiMaterial*> importedMaterials(importedScene.mMaterials, importedScene.mNumMaterials);

	for (auto* importedMaterial : importedMaterials)
	{
		std::string materialName = importedMaterial->GetName().C_Str();
		std::shared_ptr<Material> material = ProcessMaterial(graphics, importedMaterial, filePath);

		scene.AddMaterial(materialName, std::move(material));
	}
}

std::shared_ptr<Material> ModelImporter::ProcessMaterial(Graphics& graphics, aiMaterial* material, const std::string& filePath)
{
	return std::make_shared<Material>(graphics, filePath, ProcessMaterialProperties(material));
}

MaterialProperties::MaterialProperties ModelImporter::ProcessMaterialProperties(aiMaterial* material)
{
	MaterialProperties::MaterialProperties resultPropeties;

	aiString resultTexturePath = {};

	if (material->GetTexture(aiTextureType_DIFFUSE, 0, &resultTexturePath) == aiReturn_SUCCESS)
	{
		resultPropeties.hasAnyMap = true;

		resultPropeties.hasAlbedoMap = true;
		resultPropeties.albedoMapPath = std::string(resultTexturePath.data, resultTexturePath.length);
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
		//resultPropeties.hasAnyMap = true;
		//
		//resultPropeties.hasOpacityMap = true;
		//resultPropeties.opacityMapPath = std::string(resultTexturePath.data, resultTexturePath.length);
	}


	(void)material->Get(AI_MATKEY_COLOR_AMBIENT, resultPropeties.ambient); // we can ignore if the function succeded since we have this member initialized

	(void)material->Get(AI_MATKEY_COLOR_DIFFUSE, resultPropeties.albedo);

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

void ModelImporter::PushModel(Graphics& graphics, Scene& scene, float scale, aiNode* node, aiMesh** meshes, aiMaterial** materials, Model* pParent)
{
	std::vector<std::pair<aiMesh*, std::shared_ptr<Material>>> modelMeshes;
	modelMeshes.reserve(node->mNumMeshes);

	for (unsigned int meshIndex = 0; meshIndex < node->mNumMeshes; meshIndex++)
	{
		aiMesh* targetMesh = meshes[node->mMeshes[meshIndex]];
		aiString targetMaterialName = materials[targetMesh->mMaterialIndex]->GetName();

		modelMeshes.push_back({ targetMesh, scene.GetMaterial(targetMaterialName.C_Str())});
	}

	Model* pNewParent;
	{
		std::shared_ptr<Model> model = std::make_shared<Model>(graphics, pParent, node, modelMeshes, scale);
		pNewParent = model.get();

		model->SetName(std::string(node->mName.data, node->mName.length));
		scene.AddSceneObject(std::move(model));
	}

	for (unsigned int childIndex = 0; childIndex < node->mNumChildren; childIndex++)
		PushModel(graphics, scene, scale, node->mChildren[childIndex], meshes, materials, pNewParent);
}