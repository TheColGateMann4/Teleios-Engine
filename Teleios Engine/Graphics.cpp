#include "Graphics.h"
#include "Macros/ErrorMacros.h"

void Graphics::Initialize(HWND hWnd, DXGI_FORMAT colorSpace)
{
	m_colorSpace = colorSpace;

	THROW_INTERNAL_ERROR_IF("Given color space is not valid swap chain buffer format", !CheckValidColorSpace(m_colorSpace));

	// Initializing pipeline components
	{
		HRESULT hr;

#ifdef _DEBUG
		// Enabling debug layer
		{
			THROW_ERROR_NO_MSGS(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController)));

			pDebugController->EnableDebugLayer();
		}
#endif

		// Creating dxgi factory
		{
			THROW_ERROR_NO_MSGS(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&pFactory)));
		}

		// Creating device
		{
			THROW_ERROR_NO_MSGS(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice)));
		}

		// creating info queue
		{
			m_infoQueue = std::make_unique<InfoQueue>(*this);
		}

		// Creating command queue
		{
			D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
			commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			commandQueueDesc.NodeMask = 0;

			THROW_ERROR_AT_GFX_INIT(pDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&pCommandQueue)));
		}

		// Creating swap chain
		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			swapChainDesc.BufferDesc.Width = 0;
			swapChainDesc.BufferDesc.Height = 0;
			swapChainDesc.BufferDesc.Format = m_colorSpace;
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 144;
			swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 2;
			swapChainDesc.OutputWindow = hWnd;
			swapChainDesc.Windowed = TRUE;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH

			THROW_ERROR_AT_GFX_INIT(pFactory->CreateSwapChain(pCommandQueue.Get(), &swapChainDesc, &pSwapChain));
		}

		// Initializing backbuffer render target
		{
			// these buffers will loop like (frontBuffer -> first, backBuffer -> second) ---> (backBuffer -> first, frontBuffer -> second)
			// we only care about current front buffer so we know where to draw
			Microsoft::WRL::ComPtr<ID3D12Resource> pFirstBuffer, pSecondBuffer;

			pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pFirstBuffer));
			pSwapChain->GetBuffer(1, IID_PPV_ARGS(&pSecondBuffer));

			// getting width and height out of gotten render target
			{
				D3D12_RESOURCE_DESC renderTargetDesc = pFirstBuffer->GetDesc();

				m_width = renderTargetDesc.Width;
				m_height = renderTargetDesc.Height;
			}

			m_backBuffer = std::make_shared<BackBufferRenderTarget>(*this, pFirstBuffer.Get(), pSecondBuffer.Get());
		}

		// initializing graphic root signature
		m_rootSignature = std::make_unique<RootSignature>(*this);
	}
}

unsigned int Graphics::GetCurrentBackBufferIndex()
{
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3;

	pSwapChain->QueryInterface(swapChain3.GetAddressOf());

	return swapChain3->GetCurrentBackBufferIndex();
}

void Graphics::FinishFrame()
{
	pSwapChain->Present(1, NULL);
}

ID3D12Device* Graphics::GetDevice()
{
	return pDevice.Get();
}

ID3D12CommandQueue* Graphics::GetCommandQueue()
{
	return pCommandQueue.Get();
}

InfoQueue* Graphics::GetInfoQueue()
{
	return m_infoQueue.get();
}

RootSignature* Graphics::GetRootSignature()
{
	return m_rootSignature.get();
}

BackBufferRenderTarget* Graphics::GetBackBuffer()
{
	return m_backBuffer.get();
}

DepthStencilView* Graphics::GetDepthStencil()
{
	return m_depthStencilView.get();
}

DXGI_FORMAT Graphics::GetColorSpace() const noexcept
{
	return m_colorSpace;
}

unsigned int Graphics::GetWidth() const noexcept
{
	return m_width;
}

unsigned int Graphics::GetHeight() const noexcept
{
	return m_height;
}


#pragma warning(push)
#pragma warning(disable: 4061) // turning off warnings for default statement handling unhandled values by cases

constexpr bool Graphics::CheckValidColorSpace(DXGI_FORMAT format)
{
	switch(format)
	{
			// Feature level >= 9.1
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			//Feature level >= 10.0
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			//Feature level >= 11.0
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			return true;

		default:
			return false;
	}
}

#pragma warning(pop)