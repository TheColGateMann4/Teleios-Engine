#pragma once
#include "Drawable.h"

class Graphics;

class aiMesh;
class aiMaterial;
class aiMaterialProperty;

class ModelMesh : public Drawable
{
	struct MaterialPropeties
	{
		bool hasAnyMap = false;

		bool hasDiffuseMap = false;
		bool hasNormalMap = false;
		bool hasSpecularMap = false;

		std::string diffuseMapPath;
		std::string normalMapPath;
		std::string specularMapPath;
	};

public:
	ModelMesh(Graphics& graphics, aiMesh* mesh, aiMaterial* material, std::string filePath, float scale = 1.0f, DirectX::XMFLOAT3 position = { -2.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f });

	ModelMesh(const ModelMesh&) = delete;

	ModelMesh(ModelMesh&&) = delete;

private:
	MaterialPropeties ProcessMaterialPropeties(aiMaterial* material);

	template<class T>
	static void PrintMaterialPropeties(aiMaterialProperty* materialPropety)
	{
		void* baseData;
		unsigned int dataLength;
		unsigned int index;
		const char* key;

		GetMaterialPropeties(materialPropety, baseData, dataLength, index, key);

		T* pData = reinterpret_cast<T*>(baseData);
		size_t numElements = dataLength / sizeof(T);

		std::cout << "key: " << key << '\n';

		for (size_t dataIndex = 0; dataIndex < numElements; dataIndex++)
			std::cout << "data" << dataIndex << " : " << pData[dataIndex] << '\n';
	}

	static void GetMaterialPropeties(aiMaterialProperty* materialPropety, void*& baseData, unsigned int& dataLength, unsigned int& index, const char*& key);
};

