#pragma once
#include "Bindable.h"
#include "Graphics/Core/DescriptorHeap.h"

class Graphics;

class RootSignature;
class CommandList;

class GraphicsTexture;
class GraphicsBuffer;

class UnorderedAccessView : public Bindable, public RootSignatureBindable, public CommandListBindable
{
public:
	UnorderedAccessView(Graphics& graphics, GraphicsTexture* texture, unsigned int targetMip);
	UnorderedAccessView(Graphics& graphics, GraphicsBuffer* buffer, UINT slot = 0);

public:
	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual void BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList) override;

	UINT GetOffsetInDescriptor() const;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(Graphics& graphics) const;

	static DXGI_FORMAT GetTypedUAVFormat(DXGI_FORMAT format);

private:
	DescriptorHeap::DescriptorInfo m_descriptor = {};
};