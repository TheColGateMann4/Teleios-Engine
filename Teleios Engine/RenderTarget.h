#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class RenderTarget
{
protected:
	RenderTarget(Graphics& graphics, ID3D12Resource* pResource, DXGI_FORMAT format, bool isBackBuffer);

public:
	RenderTarget(Graphics& graphics, ID3D12Resource* pResource, DXGI_FORMAT format);
	RenderTarget(Graphics& graphics, DXGI_FORMAT format);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE* GetDescriptor(Graphics& graphics) const;
	virtual ID3D12Resource* GetResource(Graphics& graphics) const;

	DXGI_FORMAT GetFormat() const;

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> pRenderTarget;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptorHandle;
	UINT m_sizeOfDescriptor;
	DXGI_FORMAT m_format;
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