#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"

class Graphics;
class ConstantBuffer;
class Texture;
class StaticSampler;

class RootSignature
{
public:
	RootSignature();

public:
	ID3D12RootSignature* Get() const;

	void Initialize(Graphics& graphics);

	// returns rootIndex that was used
	void AddConstBufferViewParameter(ConstantBuffer* constantBuffer);
	void AddDescriptorTableParameter(Texture* texture);
	//srv
	// uav
	// constants

	void AddStaticSampler(StaticSampler* staticSampler);

private:
	void ConnectDescriptorParametersToRanges();

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	bool m_finished;

	D3D12_ROOT_SIGNATURE_DESC1 m_rootSignatureDesc;
	std::vector<D3D12_ROOT_PARAMETER1> m_rootParameters;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplers;
	std::vector<D3D12_DESCRIPTOR_RANGE1> m_descriptorTableRanges;
};

