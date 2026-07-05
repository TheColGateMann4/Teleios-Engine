#pragma once
#include "Bindable.h"
#include "Graphics/Data/DynamicConstantBuffer.h"

class RootSignatureConstants : public Bindable, public RootSignatureBindable
{
public:
	// data has to have finished layout with 'data' type since it is padding-less
	RootSignatureConstants(DynamicConstantBuffer::Data& data, ResourceTargets targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

public:
	virtual void AddGraphicsRootSignatureParam(RootSignatureParams* rootSignatureParams) override;

	virtual void BindToCommandListAsRootParam(Graphics& graphics, CommandList* commandList, const RootBinding& binding) override;

	virtual BindableType GetBindableType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

public:
	unsigned int GetNumValues() const;
	const void* GetDataPtr() const;

	bool IsUpdated() const;

	void SetUpdated(bool updated);

public:
	DynamicConstantBuffer::Data& GetData();

private:
	DynamicConstantBuffer::Data m_data;
	mutable bool m_isUpdated = true;
};