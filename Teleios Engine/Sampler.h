#pragma once
#include "Bindable.h"
#include "TargetShaders.h"

class Graphics;
class RootSignature;

class StaticSampler : public Bindable, public RootSignatureBindable
{
public:
	StaticSampler(D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE overlappingMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP, std::vector<TargetSlotAndShader> targets = {{ ShaderVisibilityGraphic::PixelShader, 0 }});

public:
	static std::shared_ptr<StaticSampler> GetBindableResource(D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE overlappingMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP, std::vector<TargetSlotAndShader> targets = { { ShaderVisibilityGraphic::PixelShader, 0 } });

	static std::string GetIdentifier(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE overlappingMode, std::vector<TargetSlotAndShader> targets);

public:
	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	D3D12_STATIC_SAMPLER_DESC Get() const;

	virtual std::vector<TargetSlotAndShader>& GetTargets() override;

private:
	D3D12_STATIC_SAMPLER_DESC m_staticSamplerDesc = {};
	std::vector<TargetSlotAndShader> m_targets;
};