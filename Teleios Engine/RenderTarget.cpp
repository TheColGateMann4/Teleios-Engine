#include "RenderTarget.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"

/*
			// Render Target
*/

RenderTarget::RenderTarget(Graphics& graphics, ID3D12Resource* pResource, bool isBackBuffer)
	:
	m_sizeOfDescriptor(graphics.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV))
{
	HRESULT hr;

	// saving our buffer surface for later
	pResource->QueryInterface(pRenderTarget.GetAddressOf());

	// creating descriptor for our render target view resource
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDesc.NumDescriptors = isBackBuffer ? 2 : 1; // in flip model there are two buffers
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NodeMask = 0;

		THROW_ERROR(graphics.GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&pDescriptorHeap)));
	}

	// creating render target view (if it is back buffer then we are creating first(out of two) render target view's here)
	{
		D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
		renderTargetViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D = D3D12_TEX2D_RTV{};

		graphics.GetDevice()->CreateRenderTargetView(pResource, &renderTargetViewDesc, pDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
}

RenderTarget::RenderTarget(Graphics& graphics, ID3D12Resource* pResource)
	:
	RenderTarget(graphics, pResource, false)
{

};

RenderTarget::RenderTarget(Graphics& graphics)
	:
	RenderTarget(graphics, nullptr)
{

}

const D3D12_CPU_DESCRIPTOR_HANDLE* RenderTarget::GetDescriptor(Graphics& graphics) const
{
	return &m_descriptorHandle;
}

ID3D12Resource* RenderTarget::GetResource(Graphics& graphics) const
{
	return pRenderTarget.Get();
}

/*
			// Render Target for back buffer
*/

BackBufferRenderTarget::BackBufferRenderTarget(Graphics& graphics, ID3D12Resource* pFirstBackBuffer, ID3D12Resource* pSecondBackBuffer)
	:
	RenderTarget(graphics, pFirstBackBuffer, true) // calling render target constructor with back buffer true to create descriptor with two spaces
{
	// saving our buffer surface for later
	pSecondBackBuffer->QueryInterface(pSecondRenderTarget.GetAddressOf());

	// creating second render target view
	{
		D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
		renderTargetViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D = D3D12_TEX2D_RTV{ 0,0 };

		m_secondDescriptorHandle = pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_secondDescriptorHandle.ptr += static_cast<SIZE_T>(m_sizeOfDescriptor);

		graphics.GetDevice()->CreateRenderTargetView(pSecondBackBuffer, &renderTargetViewDesc, m_secondDescriptorHandle);
	}
}

const D3D12_CPU_DESCRIPTOR_HANDLE* BackBufferRenderTarget::GetDescriptor(Graphics& graphics) const
{
	return graphics.GetCurrentBackBufferIndex() == 0 ? &m_descriptorHandle : &m_secondDescriptorHandle;
}

ID3D12Resource* BackBufferRenderTarget::GetResource(Graphics& graphics) const
{
	return graphics.GetCurrentBackBufferIndex() == 0 ? pRenderTarget.Get() : pSecondRenderTarget.Get();
}