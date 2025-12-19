#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"
#include "Bindables/Bindable.h"

class Graphics;
class ConstantBuffer;
class Texture;
class ShaderResourceViewBase;
class UnorderedAccessView;
class StaticSampler;

class RootSignature
{
public:
	RootSignature();
	RootSignature(RootSignature&& moved) noexcept;
	RootSignature(const RootSignature& copied);

public:
	ID3D12RootSignature* Get() const;

	void Initialize(Graphics& graphics);

	// returns rootIndex that was used
	void AddConstBufferViewParameter(ConstantBuffer* constantBuffer);

	void AddDescriptorTableParameter(Texture* texture);
	void AddDescriptorTableParameter(Graphics& graphics, ShaderResourceViewBase* srv);
	void AddComputeDescriptorTableParameter(Texture* texture, TargetSlotAndShader target);
	void AddComputeDescriptorTableParameter(Graphics& graphics, ShaderResourceViewBase* srv, TargetSlotAndShader target);

	void AddUnorderedAccessViewParameter(UnorderedAccessView* uav);

	// constants

	void AddStaticSampler(StaticSampler* staticSampler);
	void AddComputeStaticSampler(StaticSampler* staticSampler, TargetSlotAndShader target);

private:
	void ConnectDescriptorParametersToRanges();

	// returns root index
	unsigned int m_AddDescriptorTableParameter(UINT offsetInDescriptor, TargetSlotAndShader target, D3D12_DESCRIPTOR_RANGE_TYPE descriptorType);
	void m_AddStaticSampler(StaticSampler* staticSampler, TargetSlotAndShader target);

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	bool m_finished;

	D3D12_ROOT_SIGNATURE_DESC1 m_rootSignatureDesc;
	std::vector<D3D12_ROOT_PARAMETER1> m_rootParameters;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplers;
	std::vector<D3D12_DESCRIPTOR_RANGE1> m_descriptorTableRanges;
};

