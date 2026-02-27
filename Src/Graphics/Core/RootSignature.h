#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Shaders/TargetShaders.h"
#include "Graphics/Bindables/Bindable.h"

class Graphics;
class ConstantBuffer;
class DescriptorHeapBindable;
class Texture;
class ShaderResourceViewBase;
class UnorderedAccessView;
class RootSignatureConstants;
class StaticSampler;

class RootSignatureParams
{
public:
	RootSignatureParams();
	RootSignatureParams(RootSignatureParams&& moved) noexcept = default;
	RootSignatureParams(const RootSignatureParams& copied);

public:
	D3D12_ROOT_SIGNATURE_DESC1 GetDesc() const;

	void Finish();

	bool isFinished() const;

	std::string GetIdentifier() const;

public:
	// returns rootIndex that was used
	void AddConstBufferViewParameter(ConstantBuffer* constantBuffer, TargetSlotAndShader& target);

	void AddDescriptorTableParameter(DescriptorHeapBindable* descriptorHeapBindable);
	void AddDescriptorTableParameter(Texture* texture, TargetSlotAndShader& target);
	void AddDescriptorTableParameter(ShaderResourceViewBase* srv, TargetSlotAndShader& target);

	void AddComputeDescriptorTableParameter(Texture* texture, TargetSlotAndShader& target);
	void AddComputeDescriptorTableParameter(ShaderResourceViewBase* srv, TargetSlotAndShader& target);

	void AddUnorderedAccessViewParameter(UnorderedAccessView* uav, TargetSlotAndShader& target);

	void SetGraphicsRootConstants(RootSignatureConstants* constants, TargetSlotAndShader& target);

	void AddStaticSampler(StaticSampler* staticSampler);
	void AddComputeStaticSampler(StaticSampler* staticSampler);

private:
	void ConnectDescriptorParametersToRanges();

	// returns root index
	unsigned int m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE descriptorType, TargetSlotAndShader& target, unsigned int numDescriptors = 1, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	void m_AddStaticSampler(StaticSampler* staticSampler, TargetSlotAndShader& target);

private:
	std::string GetParamsIdentifier() const;
	std::string GetStaticSamplersIdentifier() const;
	std::string GetFlagsIdentifier() const;

	std::string GetParamIdentifier(const D3D12_ROOT_PARAMETER1& param) const;

	std::string GetParamIdentifierByType(const D3D12_ROOT_PARAMETER1& param) const;

	std::string GetDescriptorTableIdentifier(const D3D12_ROOT_DESCRIPTOR_TABLE1& descriptorTable) const;
	std::string GetConstantsIdentifier(const D3D12_ROOT_CONSTANTS& constants) const;
	std::string GetDescriptorIdentifier(const D3D12_ROOT_DESCRIPTOR1& descriptor) const;
	std::string GetStaticSamplerIdentifier(const D3D12_STATIC_SAMPLER_DESC& staticSampler) const;

private:
	bool m_finished;

	D3D12_ROOT_SIGNATURE_DESC1 m_rootSignatureDesc;
	std::vector<D3D12_ROOT_PARAMETER1> m_rootParameters;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplers;
	std::vector<D3D12_DESCRIPTOR_RANGE1> m_descriptorTableRanges;
};

class RootSignature
{
public:
	RootSignature(Graphics& graphics, RootSignatureParams&& params);

	static std::shared_ptr<RootSignature> GetResource(Graphics& graphics, RootSignatureParams&& params);

public:
	ID3D12RootSignature* Get() const;

	static std::string GetIdentifier(const RootSignatureParams& params);

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	RootSignatureParams m_params;
};

