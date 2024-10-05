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

void RenderTarget::Bind(Graphics& graphics, ID3D12GraphicsCommandList* commandList) const
{
	THROW_INFO_ERROR(commandList->OMSetRenderTargets(1, &m_descriptorHandle, 1, nullptr));
}

/*
			// Render Target for back buffer
*/

BackBufferRenderTarget::BackBufferRenderTarget(Graphics& graphics, ID3D12Resource* pFirstBackBuffer, ID3D12Resource* pSecondBackBuffer)
	:
	RenderTarget(graphics, pFirstBackBuffer, true) // calling render target constructor with back buffer true to create descriptor with two spaces
{
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

void BackBufferRenderTarget::Bind(Graphics& graphics, ID3D12GraphicsCommandList* commandList) const
{
	const D3D12_CPU_DESCRIPTOR_HANDLE* descriptorHandle = graphics.GetCurrentBackBufferIndex() == 0 ? &m_descriptorHandle : &m_secondDescriptorHandle;

	THROW_INFO_ERROR(commandList->OMSetRenderTargets(1, descriptorHandle, 1, nullptr));
}