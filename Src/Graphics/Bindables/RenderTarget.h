#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#include "Bindable.h"
#include "Graphics/Resources/GraphicsTexture.h"

class Graphics;

class RenderTarget : public Bindable, public CommandListBindable
{
protected:
	struct NonOwningRenderTargetData
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> pRenderTarget;
		D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;
		D3D12_RESOURCE_STATES state;
	};
	struct OwningRenderTargetData
	{
		D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;
		std::shared_ptr<GraphicsTexture> texture;
	};

protected:
	RenderTarget(DXGI_FORMAT format);

public:
	virtual ~RenderTarget() = default;

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE& GetDescriptor(Graphics& graphics) const = 0;
	virtual ID3D12Resource* GetResource(Graphics& graphics) const = 0;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	DXGI_FORMAT GetFormat() const;

	virtual D3D12_RESOURCE_STATES GetResourceState(Graphics& graphics) const = 0;
	virtual void SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState) = 0;

	virtual BindableType GetBindableType() const override;

protected:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
	DXGI_FORMAT m_format;
};

// RenderTarget wrapper around single resource
class SurfaceRenderTarget : public RenderTarget
{
public:
	SurfaceRenderTarget(Graphics& graphics, DXGI_FORMAT format, ID3D12Resource* pResource);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE& GetDescriptor(Graphics& graphics) const override;
	virtual ID3D12Resource* GetResource(Graphics& graphics) const override;

	virtual D3D12_RESOURCE_STATES GetResourceState(Graphics& graphics) const override;
	virtual void SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState) override;

private:
	NonOwningRenderTargetData m_renderTarget;
};

// Wrapper around back buffer resources provided by swapchain
class SwapChainRenderTarget : public RenderTarget
{
public:
	SwapChainRenderTarget(Graphics& graphics, DXGI_FORMAT format, std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>& bufferList);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE& GetDescriptor(Graphics& graphics) const override;
	virtual ID3D12Resource* GetResource(Graphics& graphics) const override;

	virtual D3D12_RESOURCE_STATES GetResourceState(Graphics& graphics) const override;
	virtual void SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState) override;

private:
	std::vector<NonOwningRenderTargetData> m_renderTargets;
};

// Owning object that creates and holds RT resources
class BackBufferRenderTarget : public RenderTarget
{
public:
	BackBufferRenderTarget(Graphics& graphics, DXGI_FORMAT format);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE& GetDescriptor(Graphics& graphics) const override;
	virtual ID3D12Resource* GetResource(Graphics& graphics) const override;
	GraphicsTexture* GetTexture(Graphics& graphics) const;
	GraphicsTexture* GetTexture(unsigned int i) const;

	virtual D3D12_RESOURCE_STATES GetResourceState(Graphics& graphics) const override;
	virtual void SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState) override;

private:
	std::vector<OwningRenderTargetData> m_ownedRenderTargets;
};