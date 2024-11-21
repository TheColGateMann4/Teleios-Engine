#include "Graphics.h"
#include "Macros/ErrorMacros.h"

void Graphics::Initialize(HWND hWnd, DXGI_FORMAT renderTargetFormat)
{
	THROW_OBJECT_STATE_ERROR_IF("Given format is not valid swap chain buffer", !CheckValidRenderTargetFormat(renderTargetFormat));

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
			UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

			THROW_ERROR_NO_MSGS(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&pFactory)));
		}

		// Creating device
		{
			THROW_ERROR_NO_MSGS(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice)));
		}

#ifdef _DEBUG
		// creating info queue
		{
			m_infoQueue = std::make_unique<InfoQueue>(*this);
		}
#endif

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
			swapChainDesc.BufferDesc.Format = renderTargetFormat;
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 144;
			swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = swapChainBufferCount;
			swapChainDesc.OutputWindow = hWnd;
			swapChainDesc.Windowed = TRUE;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH

			THROW_ERROR_AT_GFX_INIT(pFactory->CreateSwapChain(pCommandQueue.Get(), &swapChainDesc, &pSwapChain));
		}

		// Initializing backbuffer render target
		{
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> bufferList(swapChainBufferCount);

			for(unsigned int bufferIndex = 0; bufferIndex < swapChainBufferCount; bufferIndex++)
				THROW_ERROR_AT_GFX_INIT(pSwapChain->GetBuffer(bufferIndex, IID_PPV_ARGS(&bufferList.at(bufferIndex))));

			// getting width and height out of gotten render target
			{
				D3D12_RESOURCE_DESC renderTargetDesc = bufferList.front()->GetDesc();

				m_width = renderTargetDesc.Width;
				m_height = renderTargetDesc.Height;
			}

			m_backBuffer = std::make_shared<BackBufferRenderTarget>(*this, renderTargetFormat, bufferList);
		}

		// initializing depth stencil view
		m_depthStencilView = std::make_shared<DepthStencilView>(*this);

		// initializing graphic fence for each frame buffer
		for (unsigned int bufferIndex = 0; bufferIndex < swapChainBufferCount; bufferIndex++)
			m_graphicFences.push_back(Fence(*this));

		//initializing imgui
		m_imguiManager = std::make_unique<ImguiManager>(*this, hWnd);
	}
}

unsigned int Graphics::GetCurrentBufferIndex() const
{
	return m_currentBufferIndex;
}

unsigned int Graphics::GetPreviousBufferIndex() const
{
	return (m_currentBufferIndex == 0) ? GetBufferCount() - 1 : m_currentBufferIndex - 1;
}

unsigned int Graphics::GetNextBufferIndex() const
{
	return (m_currentBufferIndex >= GetBufferCount() - 1) ? 0 : m_currentBufferIndex + 1;
}

unsigned int Graphics::GetBufferCount() const
{
	return swapChainBufferCount;
}

void Graphics::BeginFrame()
{
	m_currentBufferIndex = GetCurrentBufferIndexFromSwapchain();

	m_imguiManager->BeginFrame();
}

void Graphics::FinishFrame()
{
	HRESULT hr;

	THROW_ERROR_AT_GFX_INIT(pSwapChain->Present(1, NULL));
}

void Graphics::WaitForGPU()
{
	m_graphicFences.at(GetCurrentBufferIndex()).WaitForGPU(*this);
}

void Graphics::WaitForGPUIfNextBufferInUse()
{
	m_graphicFences.at(GetPreviousBufferIndex()).WaitForValue(*this);

	m_graphicFences.at(GetCurrentBufferIndex()).SetWaitValue(*this);

	m_graphicFences.at(GetNextBufferIndex()).WaitForValue(*this);
}

ImguiManager* Graphics::GetImguiManager()
{
	return m_imguiManager.get();
}

ID3D12Device* Graphics::GetDevice()
{
	return pDevice.Get();
}

ID3D12CommandQueue* Graphics::GetCommandQueue()
{
	return pCommandQueue.Get();
}

#ifdef _DEBUG
InfoQueue* Graphics::GetInfoQueue()
{
	return m_infoQueue.get();
}
#endif

BackBufferRenderTarget* Graphics::GetBackBuffer()
{
	return m_backBuffer.get();
}

DepthStencilView* Graphics::GetDepthStencil()
{
	return m_depthStencilView.get();
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

constexpr bool Graphics::CheckValidRenderTargetFormat(DXGI_FORMAT format)
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

unsigned int Graphics::GetCurrentBufferIndexFromSwapchain()
{
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3;

	pSwapChain->QueryInterface(swapChain3.GetAddressOf());

	return swapChain3->GetCurrentBackBufferIndex();
}