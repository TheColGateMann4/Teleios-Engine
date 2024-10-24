#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class RenderTarget
{
protected:
	RenderTarget(Graphics& graphics, ID3D12Resource* pResource, DXGI_FORMAT format, bool isBackBuffer, D3D12_RESOURCE_STATES resourceState);

public:
	RenderTarget(Graphics& graphics, ID3D12Resource* pResource, DXGI_FORMAT format, D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET);
	RenderTarget(Graphics& graphics, DXGI_FORMAT format, D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE* GetDescriptor(Graphics& graphics) const;
	virtual ID3D12Resource* GetResource(Graphics& graphics) const;

	DXGI_FORMAT GetFormat() const;

	D3D12_RESOURCE_STATES GetResourceState() const;
	void SetResourceState(D3D12_RESOURCE_STATES newState);

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> pRenderTarget;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptorHandle;
	UINT m_sizeOfDescriptor;
	DXGI_FORMAT m_format;
	D3D12_RESOURCE_STATES m_state;
};

class BackBufferRenderTarget : public RenderTarget
{
public:
	BackBufferRenderTarget(Graphics& graphics, DXGI_FORMAT format, ID3D12Resource* pFirstBackBuffer, ID3D12Resource* pSecondBackBuffer);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE* GetDescriptor(Graphics& graphics) const override;
	virtual ID3D12Resource* GetResource(Graphics& graphics) const override;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pSecondRenderTarget;
	D3D12_CPU_DESCRIPTOR_HANDLE m_secondDescriptorHandle;
};