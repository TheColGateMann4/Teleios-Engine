#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"

class RootSignature
{
public:
	enum class ParameterType
	{
		DescriptorTable = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
		Bit32Constants = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
		ConstBufferView = D3D12_ROOT_PARAMETER_TYPE_CBV,
		ShaderResourceView = D3D12_ROOT_PARAMETER_TYPE_SRV,
		UnorderedAccessView = D3D12_ROOT_PARAMETER_TYPE_UAV
	};

public:
	RootSignature();

public:
	ID3D12RootSignature* Get() const;

	void Initialize(class Graphics& graphics);

	// returns rootIndex that was used
	void AddConstBufferViewParameter(class ConstantBuffer* constantBuffer);
	void AddDescriptorTableParameter(class Texture* texture);
	//srv
	// uav
	// constants

	void AddStaticSampler(UINT registerNum, TargetShader target);

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	bool m_finished;

	D3D12_ROOT_SIGNATURE_DESC m_rootSignatureDesc;
	std::vector<D3D12_ROOT_PARAMETER> m_rootParameters;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplers;
	std::vector<D3D12_DESCRIPTOR_RANGE> m_descriptorTableRanges;
};

