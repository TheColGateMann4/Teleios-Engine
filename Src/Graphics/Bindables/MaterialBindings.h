#pragma once
#include "Includes/CppIncludes.h"

#include "Bindable.h"

#include "RootSignatureConstants.h"
#include "DescriptorHeapBindable.h"

class Texture;

class MaterialBindings
{
public:
	MaterialBindings(const std::vector<Texture*>& textures);

public:
	RootSignatureConstants* GetTextureIndexesConstants();
	DescriptorHeapBindable* GetDescriptorHeapBindable();

private:
	std::shared_ptr<RootSignatureConstants> m_textureIndexesConstants;
	std::shared_ptr<DescriptorHeapBindable> m_descriptorHeapBindable;
};