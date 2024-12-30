#pragma once
#include "Bindable.h"
#include "DescriptorHeap.h"

class Graphics;
class RootSignature;
class CommandList;
class Texture;


class TextureMipView : public Bindable, public RootSignatureBindable, public CommandListBindable
{
public:
	TextureMipView(Graphics& graphics, Texture* texture, unsigned int targetMip);

public:
	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual void BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList) override;

	UINT GetOffsetInDescriptor() const;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(Graphics& graphics) const;

private:
	DescriptorHeap::DescriptorInfo m_descriptor = {};
};