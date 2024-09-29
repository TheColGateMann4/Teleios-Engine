#include "Graphics.h"
#include "Macros/ErrorMacros.h"

void Graphics::Initialize(HWND hWnd)
{
	HRESULT hr;

	// Initializing pipeline components
	{
		// Enabling debug layer
		{
			THROW_ERROR(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController)));

			pDebugController->EnableDebugLayer();
		}

		// Creating dxgi factory
		{
			THROW_ERROR(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&pFactory)));
		}

		// Creating device
		{
			THROW_ERROR(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice)));
		}

		// Creating command queue
		{
			D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
			commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			commandQueueDesc.NodeMask = 0;

			THROW_ERROR(pDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&pCommandQueue)));
		}

		// Creating swap chain
		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			swapChainDesc.BufferDesc.Width = 0;
			swapChainDesc.BufferDesc.Height = 0;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
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

			THROW_ERROR(pFactory->CreateSwapChain(pCommandQueue.Get(), &swapChainDesc, &pSwapChain));
		}

		// Creating heap descriptor
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
			heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			heapDescriptorDesc.NumDescriptors = 2;
			heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			heapDescriptorDesc.NodeMask = 0;

			THROW_ERROR(pDevice->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&pHeapDescriptor)));
		}

		// Creating command allocator
		{
			THROW_ERROR(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator)));// or D3D12_COMMAND_LIST_TYPE_BUNDLE made for groups of commands
		}

		// Creating pipeline state
		{
			//D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
			//pipelineStateDesc.
			// make it.

			//THROW_ERROR(pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pPipelineState)));
		}

		// Creating command list
		{
			pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator.Get(), pPipelineState.Get(), IID_PPV_ARGS(&pCommandList));
		}
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