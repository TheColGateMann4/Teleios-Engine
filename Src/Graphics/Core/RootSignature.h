#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Shaders/TargetShaders.h"
#include "Graphics/Bindables/Bindable.h"
#include "RootSignatureLayout.h"

class Graphics;
class ConstantBuffer;
class DescriptorHeapBindable;
class Texture;
class ShaderResourceViewBase;
class Buffer;
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

	const std::string& GetIdentifier() const;

	const RootSignatureLayout& GetLayout() const;

public:
	// returns rootIndex that was used
	void AddConstBufferViewParameter(ConstantBuffer* constantBuffer, const TargetSlotAndShader& target);

	void AddDescriptorTableParameter(DescriptorHeapBindable* descriptorHeapBindable, const TargetSlotAndShader& target);
	void AddDescriptorTableParameter(ShaderResourceViewBase* srv, const TargetSlotAndShader& target);

	void AddDescriptorParameter(Buffer* buffer, const TargetSlotAndShader& target);

	void AddUnorderedAccessViewParameter(UnorderedAccessView* uav, const TargetSlotAndShader& target);

	void SetGraphicsRootConstants(RootSignatureConstants* constants, const TargetSlotAndShader& target);

	void AddStaticSampler(StaticSampler* staticSampler, const TargetSlotAndShader& target);

private:
	void ConnectDescriptorParametersToRanges();

	void m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE descriptorType, const TargetSlotAndShader& target, unsigned int numDescriptors = 1, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	void m_AddStaticSampler(StaticSampler* staticSampler, const TargetSlotAndShader& target);

private:
	void CreateIdentifier();

	std::string GetParamsIdentifier() const;
	std::string GetStaticSamplersIdentifier() const;
	std::string GetFlagsIdentifier() const;

private:
	bool m_finished;

	D3D12_ROOT_SIGNATURE_DESC1 m_rootSignatureDesc;
	std::vector<D3D12_ROOT_PARAMETER1> m_rootParameters;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplers;
	std::vector<D3D12_DESCRIPTOR_RANGE1> m_descriptorTableRanges;

	std::string m_cachedIdentifier = {};
	RootSignatureLayout m_layout = {};
};

class RootSignature
{
public:
	RootSignature(Graphics& graphics, RootSignatureParams&& params);

	static std::shared_ptr<RootSignature> GetResource(Graphics& graphics, RootSignatureParams&& params);

public:
	ID3D12RootSignature* Get() const;

	static std::string GetIdentifier(const RootSignatureParams& params);
	std::string GetIdentifier();
	unsigned int GetNumParams() const;

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	RootSignatureParams m_params;
};