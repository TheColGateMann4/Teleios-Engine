#include "RenderTarget.h"
#include "Graphics/Core/Graphics.h"
#include "Macros/ErrorMacros.h"

#include "Graphics/Core/CommandList.h"

/*
			// Render Target
*/

RenderTarget::RenderTarget(DXGI_FORMAT format)
	:
	m_format(format)
{

}

void RenderTarget::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetRenderTarget(graphics, this, nullptr); // if we pass nullptr as DS then none is bound; valid
}

DXGI_FORMAT RenderTarget::GetFormat() const
{
	return m_format;
}

/*
			// Surface Render Target
*/

SurfaceRenderTarget::SurfaceRenderTarget(Graphics& graphics, DXGI_FORMAT format, ID3D12Resource* pResource)
	:
	RenderTarget(format)
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

	pResource->QueryInterface(m_renderTarget.pRenderTarget.GetAddressOf());

	// creating render target view
	{
		D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
		renderTargetViewDesc.Format = m_format;
		renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D = D3D12_TEX2D_RTV{};

		m_renderTarget.descriptorHandle = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		graphics.GetDevice()->CreateRenderTargetView(pResource, &renderTargetViewDesc, m_renderTarget.descriptorHandle);
	}

	m_renderTarget.state = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

const D3D12_CPU_DESCRIPTOR_HANDLE& SurfaceRenderTarget::GetDescriptor(Graphics& graphics) const
{
	return m_renderTarget.descriptorHandle;
}

ID3D12Resource* SurfaceRenderTarget::GetResource(Graphics& graphics) const
{
	return m_renderTarget.pRenderTarget.Get();
}

D3D12_RESOURCE_STATES SurfaceRenderTarget::GetResourceState(Graphics& graphics) const
{
	return m_renderTarget.state;
}

void SurfaceRenderTarget::SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState)
{
	m_renderTarget.state = newState;
}

/*
			// Wrapper around back buffer resources provided by swapchain
*/

SwapChainRenderTarget::SwapChainRenderTarget(Graphics& graphics, DXGI_FORMAT format, std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>& bufferList)
	:
	RenderTarget(format)
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
	m_renderTargets.reserve(numBuffers);

	for(auto& pResource : bufferList)
	{
		NonOwningRenderTargetData renderTargetData;

		pResource->QueryInterface(renderTargetData.pRenderTarget.GetAddressOf());

		// creating render target view
		{
			D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
			renderTargetViewDesc.Format = m_format;
			renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D = D3D12_TEX2D_RTV{};

			renderTargetData.descriptorHandle = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			renderTargetData.descriptorHandle.ptr += static_cast<SIZE_T>(accumulatedSizeOfDescriptor);

			accumulatedSizeOfDescriptor += sizeOfRTVDescriptor;

			graphics.GetDevice()->CreateRenderTargetView(renderTargetData.pRenderTarget.Get(), &renderTargetViewDesc, renderTargetData.descriptorHandle);
		}

		// setting state
		renderTargetData.state = D3D12_RESOURCE_STATE_PRESENT;

		m_renderTargets.push_back(std::move(renderTargetData));
	}
}

const D3D12_CPU_DESCRIPTOR_HANDLE& SwapChainRenderTarget::GetDescriptor(Graphics& graphics) const
{
	return m_renderTargets.at(graphics.GetCurrentBufferIndex()).descriptorHandle;
}

ID3D12Resource* SwapChainRenderTarget::GetResource(Graphics& graphics) const
{
	return m_renderTargets.at(graphics.GetCurrentBufferIndex()).pRenderTarget.Get();
}

D3D12_RESOURCE_STATES SwapChainRenderTarget::GetResourceState(Graphics& graphics) const
{
	return m_renderTargets.at(graphics.GetCurrentBufferIndex()).state;
}

void SwapChainRenderTarget::SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState)
{
	m_renderTargets.at(graphics.GetCurrentBufferIndex()).state = newState;
}

/*
			// Render Target for back buffer
*/

BackBufferRenderTarget::BackBufferRenderTarget(Graphics& graphics, DXGI_FORMAT format)
	:
	RenderTarget(format)
{
	HRESULT hr;
	
	const unsigned int numBuffers = graphics.GetBufferCount();

	UINT accumulatedSizeOfDescriptor = 0;
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

	static constexpr DirectX::XMFLOAT4 optimizedClearValue = { 0.01f, 0.02f, 0.03f, 1.0f };
	
	m_ownedRenderTargets.reserve(graphics.GetBufferCount());

	for (int i = 0; i < graphics.GetBufferCount(); i++)
	{
		OwningRenderTargetData renderTargetData;

		renderTargetData.texture = std::make_shared<GraphicsTexture>(graphics, graphics.GetWidth(), graphics.GetHeight(), 1, format, optimizedClearValue, GraphicsResource::CPUAccess::notavailable, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		// creating render target view
		{
			D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
			renderTargetViewDesc.Format = m_format;
			renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D = D3D12_TEX2D_RTV{};

			renderTargetData.descriptorHandle = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			renderTargetData.descriptorHandle.ptr += static_cast<SIZE_T>(accumulatedSizeOfDescriptor);

			accumulatedSizeOfDescriptor += sizeOfRTVDescriptor;

			graphics.GetDevice()->CreateRenderTargetView(renderTargetData.texture->GetResource(), &renderTargetViewDesc, renderTargetData.descriptorHandle);
		}

		m_ownedRenderTargets.push_back(std::move(renderTargetData));
	}
}

const D3D12_CPU_DESCRIPTOR_HANDLE& BackBufferRenderTarget::GetDescriptor(Graphics& graphics) const
{
	return m_ownedRenderTargets.at(graphics.GetCurrentBufferIndex()).descriptorHandle;
}

ID3D12Resource* BackBufferRenderTarget::GetResource(Graphics& graphics) const
{
	return m_ownedRenderTargets.at(graphics.GetCurrentBufferIndex()).texture->GetResource();
}

GraphicsTexture* BackBufferRenderTarget::GetTexture(Graphics& graphics) const
{
	return GetTexture(graphics.GetCurrentBufferIndex());
}

GraphicsTexture* BackBufferRenderTarget::GetTexture(unsigned int i) const
{
	return m_ownedRenderTargets.at(i).texture.get();
}

D3D12_RESOURCE_STATES BackBufferRenderTarget::GetResourceState(Graphics& graphics) const
{
	return m_ownedRenderTargets.at(graphics.GetCurrentBufferIndex()).texture->GetResourceState();
}

void BackBufferRenderTarget::SetResourceState(Graphics& graphics, D3D12_RESOURCE_STATES newState)
{
	m_ownedRenderTargets.at(graphics.GetCurrentBufferIndex()).texture->SetResourceState(newState);
}