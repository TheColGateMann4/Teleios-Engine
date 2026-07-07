#pragma once
#include "Graphics/Core/DescriptorHeap.h"
#include "Bindable.h"

class Graphics;

class RootSignature;
class CommandList;

class GraphicsTexture;
class GraphicsBuffer;

class UnorderedAccessView : public Bindable, public RootSignatureBindable, public CommandListBindable, public DescriptorBindable
{
public:
	UnorderedAccessView(Graphics& graphics, GraphicsTexture* texture, unsigned int targetMip);
	UnorderedAccessView(Graphics& graphics, GraphicsBuffer* buffer, UINT slot = 0);

public:
	virtual void AddGraphicsRootSignatureParam(RootSignatureParams* rootSignatureParams) override;

	virtual void BindToCommandListAsRootParam(Graphics& graphics, CommandList* commandList, const RootBinding& binding) override;

	virtual BindableType GetBindableType() const override;

	virtual DescriptorType GetDescriptorType() const override;


	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

	UINT GetOffsetInDescriptor(Graphics& graphics) const;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(Graphics& graphics) const;

	static DXGI_FORMAT GetTypedUAVFormat(DXGI_FORMAT format);

private:
	unsigned int m_descriptorIndex = {};
};
