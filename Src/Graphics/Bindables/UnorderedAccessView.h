#pragma once
#include "Binding.h"
#include "Graphics/Core/DescriptorHeap.h"

class Graphics;

class RootSignature;
class CommandList;

class GraphicsTexture;
class GraphicsBuffer;

class UnorderedAccessView : public Bindable, public RootParameterBinding
{
public:
	UnorderedAccessView(Graphics& graphics, GraphicsTexture* texture, unsigned int targetMip);
	UnorderedAccessView(Graphics& graphics, GraphicsBuffer* buffer, UINT slot = 0);

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target) override;

	virtual void BindToComputeRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target) override;

	virtual BindableType GetBindableType() const override;

	UINT GetOffsetInDescriptor() const;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(Graphics& graphics) const;

	static DXGI_FORMAT GetTypedUAVFormat(DXGI_FORMAT format);

private:
	DescriptorHeap::DescriptorInfo m_descriptor = {};
};
