#pragma once
#include "Binding.h"
#include "Graphics/Core/DescriptorHeap.h"

class Graphics;
class GraphicsTexture;
class GraphicsBuffer;
class GraphicsResource;
class RootSignature;
class CommandList;

class BackBufferRenderTarget;
class DepthStencilViewMultiResource;

class ShaderResourceViewBase : public Bindable, public RootParameterBinding, public DescriptorBindable
{
protected:
	ShaderResourceViewBase(unsigned int slot);
	ShaderResourceViewBase(ShaderResourceViewBase&&) noexcept = default;

public:
	virtual unsigned int GetOffsetInDescriptor(Graphics& graphics) const = 0;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToRootSignature(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target) override;

	virtual void AddComputeRootSignatureParam(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target) override;

	virtual BindableType GetBindableType() const override;

	virtual DescriptorType GetDescriptorType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

public:
	void SetComputeRootIndex(unsigned int rootIndex);

	unsigned int GetComputeRootIndex() const;

protected:
	static void InitializeTextureSRV(Graphics& graphics, unsigned int targetMip, DescriptorHeap::DescriptorInfo& descriptor, const GraphicsTexture* texture);
	static void InitializeBufferSRV(Graphics& graphics, DescriptorHeap::DescriptorInfo& descriptor, const GraphicsBuffer* buffer);

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

	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) override;

	virtual void Initialize(Graphics& graphics) override;

private:
	unsigned int m_targetSubresource = 0;
	GraphicsResource* m_resource;
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

	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) override;

	virtual void Initialize(Graphics& graphics) override;

public:
	static std::shared_ptr<ShaderResourceViewMultiResource> GetResource(Graphics& graphics, std::string identifier, BackBufferRenderTarget* renderTarget, UINT slot = 0);
	static std::shared_ptr<ShaderResourceViewMultiResource> GetResource(Graphics& graphics, std::string identifier, DepthStencilViewMultiResource* depthStencil, UINT slot = 0);

	static std::string GetIdentifier(std::string identifier);

	GraphicsResource* GetResource(Graphics& graphics) const;

private:
	std::vector<GraphicsResource*> m_resources;
	std::vector<DescriptorHeap::DescriptorInfo> m_descriptors = {};
};
