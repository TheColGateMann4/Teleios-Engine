#pragma once
#include "Bindable.h"
#include "Graphics.h"

class GraphicsTexture;
class GraphicsBuffer;
class RootSignature;
class CommandList;

class BackBufferRenderTarget;
class DepthStencilViewMultiResource;

class ShaderResourceViewBase : public Bindable, public CommandListBindable, public RootSignatureBindable
{
public:
	ShaderResourceViewBase(unsigned int slot);
	ShaderResourceViewBase(ShaderResourceViewBase&&) noexcept = default;

public:
	virtual unsigned int GetOffsetInDescriptor(Graphics& graphics) const = 0;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual void BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

public:
	void SetComputeRootIndex(unsigned int rootIndex);

	unsigned int GetComputeRootIndex() const;

protected:
	static void InitializeTextureSRV(Graphics& graphics, unsigned int targetMip, DescriptorHeap::DescriptorInfo& descriptor, const GraphicsTexture* texture);
	static void InitializeBufferSRV(Graphics& graphics, DescriptorHeap::DescriptorInfo& descriptor, GraphicsBuffer* buffer);

protected:
	unsigned int m_computeRootIndex;
};

class ShaderResourceView : public ShaderResourceViewBase
{
public:
	ShaderResourceView(Graphics& graphics, GraphicsTexture* texture, unsigned int targetMip, UINT slot = 0);
	ShaderResourceView(Graphics& graphics, GraphicsBuffer* buffer, UINT slot = 0);
	ShaderResourceView(ShaderResourceView&&) noexcept = default;

public:
	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	virtual unsigned int GetOffsetInDescriptor(Graphics& graphics) const override;

private:
	DescriptorHeap::DescriptorInfo m_descriptor = {};
};

class ShaderResourceViewMultiResource : public ShaderResourceViewBase
{
public:
	ShaderResourceViewMultiResource(Graphics& graphics, BackBufferRenderTarget* renderTarget, UINT slot = 0);
	ShaderResourceViewMultiResource(Graphics& graphics, DepthStencilViewMultiResource* depthStencil, UINT slot = 0);
	ShaderResourceViewMultiResource(ShaderResourceViewMultiResource&&) noexcept = default;

public:
	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	virtual unsigned int GetOffsetInDescriptor(Graphics& graphics) const override;

public:
	static std::shared_ptr<ShaderResourceViewMultiResource> GetBindableResource(Graphics& graphics, std::string identifier, BackBufferRenderTarget* renderTarget, UINT slot = 0);
	static std::shared_ptr<ShaderResourceViewMultiResource> GetBindableResource(Graphics& graphics, std::string identifier, DepthStencilViewMultiResource* depthStencil, UINT slot = 0);

	static std::string GetIdentifier(std::string identifier);

private:
	std::vector<DescriptorHeap::DescriptorInfo> m_descriptors = {};
};