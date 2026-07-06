#pragma once
#include "Bindable.h"
#include "Shaders/TargetShaders.h"

class Graphics;
class RootSignature;

class StaticSampler : public Bindable, public RootSignatureBindable
{
public:
	StaticSampler(Graphics& graphics, D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE overlappingMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP, ResourceTargets targets = {{ ShaderVisibilityGraphic::PixelShader, 0 }});

public:
	static std::shared_ptr<StaticSampler> GetResource(Graphics& graphics, D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE overlappingMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP, ResourceTargets targets = { { ShaderVisibilityGraphic::PixelShader, 0 } });

	static std::string GetIdentifier(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE overlappingMode, ResourceTargets targets);

public:
	virtual void AddGraphicsRootSignatureParam(RootSignatureParams* rootSignatureParams) override;

	virtual void BindToCommandListAsRootParam(Graphics& graphics, CommandList* commandList, const RootBinding& binding) override;

	virtual BindableType GetBindableType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

	D3D12_STATIC_SAMPLER_DESC Get() const;

private:
	D3D12_STATIC_SAMPLER_DESC m_staticSamplerDesc = {};
};