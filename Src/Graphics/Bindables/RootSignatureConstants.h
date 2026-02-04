#pragma once
#include "Binding.h"

#include "Graphics/Data/DynamicConstantBuffer.h"

class RootSignatureConstants : public Bindable, public RootParameterBinding
{
public:
	RootSignatureConstants(DynamicConstantBuffer::Data& data, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target) override;

	virtual BindableType GetBindableType() const override;

public:
	unsigned int GetNumValues() const;
	const void* GetDataPtr() const;

public:
	DynamicConstantBuffer::Data& GetData();

private:
	DynamicConstantBuffer::Data m_data;
};