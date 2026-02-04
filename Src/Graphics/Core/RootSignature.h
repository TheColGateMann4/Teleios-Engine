#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Shaders/TargetShaders.h"
#include "Graphics/Bindables/Bindable.h"

class Graphics;
class ConstantBuffer;
class Texture;
class ShaderResourceViewBase;
class UnorderedAccessView;
class RootSignatureConstants;
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
	void AddConstBufferViewParameter(ConstantBuffer* constantBuffer, TargetSlotAndShader& target);

	void AddDescriptorTableParameter(Texture* texture, TargetSlotAndShader& target);
	void AddDescriptorTableParameter(ShaderResourceViewBase* srv, TargetSlotAndShader& target);

	void AddComputeDescriptorTableParameter(Texture* texture, TargetSlotAndShader& target);
	void AddComputeDescriptorTableParameter(ShaderResourceViewBase* srv, TargetSlotAndShader& target);

	void AddUnorderedAccessViewParameter(UnorderedAccessView* uav, TargetSlotAndShader& target);

	// constants

	void AddStaticSampler(StaticSampler* staticSampler);
	void AddComputeStaticSampler(StaticSampler* staticSampler);

private:
	void ConnectDescriptorParametersToRanges();

	// returns root index
	unsigned int m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE descriptorType, TargetSlotAndShader& target);
	void m_AddStaticSampler(StaticSampler* staticSampler, TargetSlotAndShader& target);

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	bool m_finished;

	D3D12_ROOT_SIGNATURE_DESC1 m_rootSignatureDesc;
	std::vector<D3D12_ROOT_PARAMETER1> m_rootParameters;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplers;
	std::vector<D3D12_DESCRIPTOR_RANGE1> m_descriptorTableRanges;
};

