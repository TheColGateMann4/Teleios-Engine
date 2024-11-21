#include "RenderTarget.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"

/*
			// Render Target
*/

SurfaceRenderTarget::SurfaceRenderTarget(Graphics& graphics, DXGI_FORMAT format, ID3D12Resource* pResource)
	:
	m_format(format)
{
	HRESULT hr;

	// creating descriptor for RTV
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDesc.NumDescriptors = 1;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NodeMask = 0;

		THROW_ERROR(graphics.GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeap)));
	}

	pResource->QueryInterface(m_renderTargetView.pRenderTarget.GetAddressOf());

	// creating render target view
	{
		D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
		renderTargetViewDesc.Format = m_format;
		renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D = D3D12_TEX2D_RTV{};

		m_renderTargetView.descriptorHandle = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		graphics.GetDevice()->CreateRenderTargetView(pResource, &renderTargetViewDesc, m_renderTargetView.descriptorHandle);
	}

	m_renderTargetView.state = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

const D3D12_CPU_DESCRIPTOR_HANDLE* SurfaceRenderTarget::GetDescriptor(Graphics& graphics) const
{
	return &m_renderTargetView.descriptorHandle;
}

ID3D12Resource* SurfaceRenderTarget::GetResource(Graphics& graphics) const
{
	return m_renderTargetView.pRenderTarget.Get();
}

DXGI_FORMAT SurfaceRenderTarget::GetFormat() const
{
	return m_format;
}

D3D12_RESOURCE_STATES SurfaceRenderTarget::GetResourceState(Graphics& graphics) const
{
	return m_renderTargetView.state;
}

void SurfaceRenderTarget::SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState)
{
	m_renderTargetView.state = newState;
}

/*
			// Render Target for back buffer
*/

BackBufferRenderTarget::BackBufferRenderTarget(Graphics& graphics, DXGI_FORMAT format, std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>& bufferList)
	:
	m_format(format)
{
	HRESULT hr;

	UINT accumulatedSizeOfDescriptor = 0;
	const size_t numBuffers = bufferList.size();
	static const UINT sizeOfRTVDescriptor = graphics.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// creating descriptor for all RTV's
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDesc.NumDescriptors = numBuffers;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NodeMask = 0;

		THROW_ERROR(graphics.GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeap)));
	}


	// getting space in vector before the pushes
	m_renderTargetViews.reserve(numBuffers);

	for(auto& pResource : bufferList)
	{
		RenderTargetView renderTargetView;

		pResource->QueryInterface(renderTargetView.pRenderTarget.GetAddressOf());

		// creating render target view
		{
			D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
			renderTargetViewDesc.Format = m_format;
			renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D = D3D12_TEX2D_RTV{};

			renderTargetView.descriptorHandle = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			renderTargetView.descriptorHandle.ptr += static_cast<SIZE_T>(accumulatedSizeOfDescriptor);

			accumulatedSizeOfDescriptor += sizeOfRTVDescriptor;

			graphics.GetDevice()->CreateRenderTargetView(renderTargetView.pRenderTarget.Get(), &renderTargetViewDesc, renderTargetView.descriptorHandle);
		}

		// setting state
		renderTargetView.state = D3D12_RESOURCE_STATE_PRESENT;

		m_renderTargetViews.push_back(std::move(renderTargetView));
	}
}


const D3D12_CPU_DESCRIPTOR_HANDLE* BackBufferRenderTarget::GetDescriptor(Graphics& graphics) const
{
	return &m_renderTargetViews.at(graphics.GetCurrentBufferIndex()).descriptorHandle;
}

ID3D12Resource* BackBufferRenderTarget::GetResource(Graphics& graphics) const
{
	return m_renderTargetViews.at(graphics.GetCurrentBufferIndex()).pRenderTarget.Get();
}

DXGI_FORMAT BackBufferRenderTarget::GetFormat() const
{
	return m_format;
}

D3D12_RESOURCE_STATES BackBufferRenderTarget::GetResourceState(Graphics& graphics) const
{
	return m_renderTargetViews.at(graphics.GetCurrentBufferIndex()).state;
}

void BackBufferRenderTarget::SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState)
{
	m_renderTargetViews.at(graphics.GetCurrentBufferIndex()).state = newState;
}