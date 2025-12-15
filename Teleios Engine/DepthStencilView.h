#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class DepthStencilView
{
public:
	DepthStencilView(class Graphics& graphics);

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDescriptor() const;

	DXGI_FORMAT GetFormat() const;

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilSurface;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
	DXGI_FORMAT m_format;
};

