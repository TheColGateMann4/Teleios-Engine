#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class RenderTarget
{
protected: 
	struct RenderTargetView
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> pRenderTarget;
		D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;
		D3D12_RESOURCE_STATES state;
	};

public:
	virtual ~RenderTarget() = default;

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE* GetDescriptor(Graphics& graphics) const = 0;
	virtual ID3D12Resource* GetResource(Graphics& graphics) const = 0;

	virtual DXGI_FORMAT GetFormat() const = 0;

	virtual D3D12_RESOURCE_STATES GetResourceState(Graphics& graphics) const = 0;
	virtual void SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState) = 0;
};

class SurfaceRenderTarget : public RenderTarget
{
public:
	SurfaceRenderTarget(Graphics& graphics, DXGI_FORMAT format, ID3D12Resource* pResource);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE* GetDescriptor(Graphics& graphics) const override;
	virtual ID3D12Resource* GetResource(Graphics& graphics) const override;

	virtual DXGI_FORMAT GetFormat() const override;

	virtual D3D12_RESOURCE_STATES GetResourceState(Graphics& graphics) const override;
	virtual void SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState) override;

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
	RenderTargetView m_renderTargetView;
	DXGI_FORMAT m_format;
};

class BackBufferRenderTarget : public RenderTarget
{
public:
	BackBufferRenderTarget(Graphics& graphics, DXGI_FORMAT format, std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>& bufferList);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE* GetDescriptor(Graphics& graphics) const override;
	virtual ID3D12Resource* GetResource(Graphics& graphics) const override;

	virtual DXGI_FORMAT GetFormat() const override;

	virtual D3D12_RESOURCE_STATES GetResourceState(Graphics& graphics) const override;
	virtual void SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState) override;

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
	std::vector<RenderTargetView> m_renderTargetViews;
	DXGI_FORMAT m_format;
};