#pragma once
#include "Includes/CppIncludes.h"

#include "Bindable.h"

#include "RootSignatureConstants.h"
#include "DescriptorHeapBindable.h"

class Texture;

class MaterialBindings : public Bindable, public RootSignatureBindable, public CommandListBindable, public DescriptorBindable
{
public:
	MaterialBindings();

public:
	virtual void BindToRootSignature(RootSignatureParams* rootSignatureParams) override;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual BindableType GetBindableType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) override;

	virtual void Initialize(Graphics& graphics) override;

	virtual DescriptorType GetDescriptorType() const override;

public:
	void InitializeTextureIndexesConstants(std::vector<Texture*> textures);

public:
	std::shared_ptr<RootSignatureConstants> m_textureIndexesConstants;
	std::shared_ptr<DescriptorHeapBindable> m_descriptorHeapBindable;
};