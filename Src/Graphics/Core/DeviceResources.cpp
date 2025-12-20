#include "DeviceResources.h"
#include "Macros/ErrorMacros.h"

#include "Graphics.h"

void DeviceResources::InitializeEntryResources()
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
}

void DeviceResources::InitializeGraphicsResources(Graphics& graphics, HWND hWnd, DXGI_FORMAT format, unsigned int bufferCount)
{
	HRESULT hr;

	// Creating command queue
	{
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.NodeMask = 0;

		THROW_INFO_ERROR(pDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&pCommandQueue)));
	}

	// Creating swap chain
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc.Width = 0;
		swapChainDesc.BufferDesc.Height = 0;
		swapChainDesc.BufferDesc.Format = format;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 144;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = bufferCount;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = 0;

		THROW_INFO_ERROR(pFactory->CreateSwapChain(pCommandQueue.Get(), &swapChainDesc, &pSwapChain));
	}
}

IDXGIFactory2* DeviceResources::GetFactory()
{
	return pFactory.Get();
}

ID3D12Debug6* DeviceResources::GetDebugController()
{
	return pDebugController.Get();
}

ID3D12Device10* DeviceResources::GetDevice()
{
	return pDevice.Get();
}

ID3D12CommandQueue* DeviceResources::GetCommandQueue()
{
	return pCommandQueue.Get();
}

IDXGISwapChain* DeviceResources::GetSwapChain()
{
	return pSwapChain.Get();
}
