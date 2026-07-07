#pragma once
#include "Graphics/Core/DescriptorHeap.h"
#include "Bindable.h"

class Graphics;
class GraphicsTexture;
class GraphicsBuffer;
class GraphicsResource;
class RootSignature;
class CommandList;

class BackBufferRenderTarget;
class DepthStencilViewMultiResource;
class DepthStencilViewCubeMultiResource;

class ShaderResourceViewBase : public Bindable, public RootSignatureBindable, public DescriptorBindable
{
protected:
	ShaderResourceViewBase(unsigned int slot);
	ShaderResourceViewBase(ShaderResourceViewBase&&) noexcept = default;

public:
	virtual unsigned int GetOffsetInDescriptor(Graphics& graphics) const = 0;

	virtual void AddGraphicsRootSignatureParam(RootSignatureParams* rootSignatureParams) override;

	virtual void BindToCommandListAsRootParam(Graphics& graphics, CommandList* commandList, const RootBinding& binding) override;

	virtual BindableType GetBindableType() const override;

	virtual DescriptorType GetDescriptorType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

protected:
	static void InitializeTextureSRV(Graphics& graphics, unsigned int targetMip, DescriptorHeap::DescriptorInfo& descriptor, const GraphicsTexture* texture);
	static void InitializeBufferSRV(Graphics& graphics, DescriptorHeap::DescriptorInfo& descriptor, const GraphicsBuffer* buffer);
};

class ShaderResourceView : public ShaderResourceViewBase
{
public:
	ShaderResourceView(Graphics& graphics, GraphicsTexture* texture, unsigned int targetMip, UINT slot = 0);
	ShaderResourceView(Graphics& graphics, GraphicsBuffer* buffer, UINT slot = 0);
	ShaderResourceView(ShaderResourceView&&) noexcept = default;

public:
	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

	virtual unsigned int GetOffsetInDescriptor(Graphics& graphics) const override;

	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) override;

	virtual void Initialize(Graphics& graphics) override;

private:
	unsigned int m_targetSubresource = 0;
	GraphicsResource* m_resource;
	unsigned int m_descriptorIndex = 0;
};

class ShaderResourceViewMultiResource : public ShaderResourceViewBase
{
public:
	ShaderResourceViewMultiResource(Graphics& graphics, BackBufferRenderTarget* renderTarget, UINT slot = 0);
	ShaderResourceViewMultiResource(Graphics& graphics, DepthStencilViewMultiResource* depthStencil, UINT slot = 0);
	ShaderResourceViewMultiResource(Graphics& graphics, DepthStencilViewCubeMultiResource* depthStencil, UINT slot = 0);
	ShaderResourceViewMultiResource(ShaderResourceViewMultiResource&&) noexcept = default;

public:
	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

	virtual unsigned int GetOffsetInDescriptor(Graphics& graphics) const override;

	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) override;

	virtual void Initialize(Graphics& graphics) override;

public:
	static std::shared_ptr<ShaderResourceViewMultiResource> GetResource(Graphics& graphics, std::string identifier, BackBufferRenderTarget* renderTarget, UINT slot = 0);
	static std::shared_ptr<ShaderResourceViewMultiResource> GetResource(Graphics& graphics, std::string identifier, DepthStencilViewMultiResource* depthStencil, UINT slot = 0);

	static std::string GetIdentifier(std::string identifier);

	GraphicsResource* GetResource(Graphics& graphics) const;

private:
	std::vector<GraphicsResource*> m_resources;
	std::vector<unsigned int> m_descriptors = {};
};
