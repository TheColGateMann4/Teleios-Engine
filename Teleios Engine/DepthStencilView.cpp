#include "DepthStencilView.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

DepthStencilView::DepthStencilView(Graphics& graphics)
	:
	m_format(DXGI_FORMAT_D24_UNORM_S8_UINT) // 24 bytes of depth, and 8 bytes of stencil
{
	HRESULT hr;

	// creating resource
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapPropeties.CreationNodeMask = 0;
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = graphics.GetWidth();
		resourceDesc.Height = graphics.GetHeight();
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = m_format;
		resourceDesc.SampleDesc = DXGI_SAMPLE_DESC{1, 0};
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = m_format;
		clearValue.DepthStencil = {1.0f, 0}; // depth 1.0f, stencil 0

		THROW_ERROR(graphics.GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&m_depthStencilSurface)
		));
	}

	// creating desriptor heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descriptorHeapDesc.NumDescriptors = 1;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NodeMask = 0;

		THROW_ERROR(graphics.GetDevice()->CreateDescriptorHeap(
			&descriptorHeapDesc,
			IID_PPV_ARGS(&m_descriptorHeap)
		));

		m_descriptor = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	}

	// creating view
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = m_format;
		depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		THROW_INFO_ERROR(graphics.GetDevice()->CreateDepthStencilView(
			m_depthStencilSurface.Get(),
			&depthStencilViewDesc,
			m_descriptor
		));
	}
}

const D3D12_CPU_DESCRIPTOR_HANDLE& DepthStencilView::GetDescriptor() const
{
	return m_descriptor;
}

DXGI_FORMAT DepthStencilView::GetFormat() const
{
	return m_format;
}