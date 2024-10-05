#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class RenderTarget
{
protected:
	RenderTarget(Graphics& graphics, ID3D12Resource* pResource, bool isBackBuffer);

public:
	RenderTarget(Graphics& graphics, ID3D12Resource* pResource);
	RenderTarget(Graphics& graphics);

public:
	void Bind(Graphics& graphics, ID3D12GraphicsCommandList* commandList) const;

protected:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptorHandle;
	UINT m_sizeOfDescriptor;
};

class BackBufferRenderTarget : public RenderTarget
{
public:
	BackBufferRenderTarget(Graphics& graphics, ID3D12Resource* pFirstBackBuffer, ID3D12Resource* pSecondBackBuffer);

public:
	void Bind(Graphics& graphics, ID3D12GraphicsCommandList* commandList) const;

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_secondDescriptorHandle;
};