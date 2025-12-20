#include "DepthStencilView.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/Graphics.h"

DepthStencilViewBase::DepthStencilViewBase(Graphics& graphics, unsigned int numDescriptors)
{
	HRESULT hr;

	// creating desriptor heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descriptorHeapDesc.NumDescriptors = numDescriptors;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NodeMask = 0;

		THROW_ERROR(graphics.GetDeviceResources().GetDevice()->CreateDescriptorHeap(
			&descriptorHeapDesc,
			IID_PPV_ARGS(&m_descriptorHeap)
		));
	}
}

void DepthStencilViewBase::CreateDSV(Graphics& graphics, D3D12_CPU_DESCRIPTOR_HANDLE& descriptor, GraphicsTexture* texture)
{
	// creating view
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = texture->GetFormat();
		depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		THROW_INFO_ERROR(graphics.GetDeviceResources().GetDevice()->CreateDepthStencilView(
			texture->GetResource(),
			&depthStencilViewDesc,
			descriptor
		));
	}
}

DepthStencilView::DepthStencilView(Graphics& graphics)
	:
	DepthStencilViewBase(graphics, 1),
	m_texture(graphics, graphics.GetWidth(), graphics.GetHeight(), 1, DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0, GraphicsResource::CPUAccess::notavailable, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
{
	CreateDSV(graphics, m_descriptor, &m_texture);
}

const D3D12_CPU_DESCRIPTOR_HANDLE& DepthStencilView::GetDescriptor(Graphics& graphics) const
{
	return m_descriptor;
}

const GraphicsTexture* DepthStencilView::GetResource(Graphics& graphics) const
{
	return &m_texture;
}

GraphicsTexture* DepthStencilView::GetResource(Graphics& graphics)
{
	return &m_texture;
}

DepthStencilViewMultiResource::DepthStencilViewMultiResource(Graphics& graphics)
	:
	DepthStencilViewBase(graphics, graphics.GetBufferCount())
{
	unsigned int numBuffers = graphics.GetBufferCount();

	m_textures.reserve(numBuffers);
	m_descriptors.resize(numBuffers);

	for(int i = 0; i < numBuffers; i++)
	{
		m_textures.push_back(std::make_shared<GraphicsTexture>(graphics, graphics.GetWidth(), graphics.GetHeight(), 1, DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0, GraphicsResource::CPUAccess::notavailable, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));
		
		D3D12_CPU_DESCRIPTOR_HANDLE& descriptor = m_descriptors.at(i);
		descriptor = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		descriptor.ptr += graphics.GetDeviceResources().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV) * i;
		
		CreateDSV(graphics, descriptor, m_textures.at(i).get());
	}

}

const D3D12_CPU_DESCRIPTOR_HANDLE& DepthStencilViewMultiResource::GetDescriptor(Graphics& graphics) const
{
	return m_descriptors.at(graphics.GetCurrentBufferIndex());
}

const GraphicsTexture* DepthStencilViewMultiResource::GetResource(Graphics& graphics) const
{
	return GetResource(graphics.GetCurrentBufferIndex());
}

GraphicsTexture* DepthStencilViewMultiResource::GetResource(Graphics& graphics)
{
	return GetResource(graphics.GetCurrentBufferIndex());
}

const GraphicsTexture* DepthStencilViewMultiResource::GetResource(unsigned int i) const
{
	return m_textures.at(i).get();
}

GraphicsTexture* DepthStencilViewMultiResource::GetResource(unsigned int i)
{
	return m_textures.at(i).get();
}