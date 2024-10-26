#include "Sampler.h"
#include "RootSignature.h"

StaticSampler::StaticSampler(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE overlappingMode, std::vector<TargetSlotAndShader> targets)
	: 
	m_targets(targets)
{
	m_staticSamplerDesc.Filter = filter;
	m_staticSamplerDesc.AddressU = overlappingMode;
	m_staticSamplerDesc.AddressV = overlappingMode;
	m_staticSamplerDesc.AddressW = overlappingMode;
	m_staticSamplerDesc.MipLODBias = 0;
	m_staticSamplerDesc.MaxAnisotropy = 16;
	m_staticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	m_staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	m_staticSamplerDesc.MinLOD = 0;
	m_staticSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	m_staticSamplerDesc.ShaderRegister = targets.front().slot;
	m_staticSamplerDesc.RegisterSpace = 0;
	m_staticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY(targets.front().target);
}

void StaticSampler::BindToRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddStaticSampler(this);
}

D3D12_STATIC_SAMPLER_DESC StaticSampler::Get() const
{
	return m_staticSamplerDesc;
}

std::vector<TargetSlotAndShader>& StaticSampler::GetTargets()
{
	return m_targets;
}