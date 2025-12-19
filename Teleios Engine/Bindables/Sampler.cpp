#include "Sampler.h"
#include "RootSignature.h"

#include "BindableResourceList.h"

StaticSampler::StaticSampler(Graphics& graphics, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE overlappingMode, std::vector<TargetSlotAndShader> targets)
	: 
	RootSignatureBindable(targets)
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
	m_staticSamplerDesc.RegisterSpace = 0;
}

std::shared_ptr<StaticSampler> StaticSampler::GetBindableResource(Graphics& graphics, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE overlappingMode, std::vector<TargetSlotAndShader> targets)
{
	return BindableResourceList::GetBindableResource<StaticSampler>(graphics, filter, overlappingMode, targets);
}

std::string StaticSampler::GetIdentifier(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE overlappingMode, std::vector<TargetSlotAndShader> targets)
{
	std::string resultString = "StaticSampler#";

	resultString += std::to_string(size_t(filter));
	resultString += '#';

	resultString += std::to_string(size_t(overlappingMode));
	resultString += '#';

	for (const auto target : targets)
	{
		resultString += target.slot;
		resultString += '#';

		resultString += std::to_string(size_t(target.target));
		resultString += '#';
	}

	return resultString;
}

void StaticSampler::BindToRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddStaticSampler(this);
}

void StaticSampler::BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddComputeStaticSampler(this, { ShaderVisibilityGraphic::AllShaders, 0 });
}

D3D12_STATIC_SAMPLER_DESC StaticSampler::Get() const
{
	return m_staticSamplerDesc;
}