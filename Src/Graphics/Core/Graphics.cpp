#include "Graphics.h"
#include "Macros/ErrorMacros.h"

#include "Graphics/Core/Pix.h"
#include "ResourceList.h"
#include "Includes/DirectXIncludes.h"

extern "C"
{
	__declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_PREVIEW_SDK_VERSION;

	__declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

Graphics::Graphics(HWND hWnd, DXGI_FORMAT renderTargetFormat)
	:
	m_windowHwnd(hWnd)
{
	THROW_OBJECT_STATE_ERROR_IF("Given format is not valid swap chain buffer", !CheckValidRenderTargetFormat(renderTargetFormat));

	// Initializing pipeline components
	{
		HRESULT hr;

		deviceResources.InitializeEntryResources();

#ifdef _DEBUG
		// creating info queue
		{
			m_infoQueue = std::make_unique<InfoQueue>(*this);
		}
#endif

		deviceResources.InitializeGraphicsResources(*this, hWnd, renderTargetFormat, swapChainBufferCount);

		// Initializing swapchain holding RenderTarget class
		{
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> bufferList(swapChainBufferCount);

			for (unsigned int bufferIndex = 0; bufferIndex < swapChainBufferCount; bufferIndex++)
				THROW_ERROR_AT_GFX_INIT(deviceResources.GetSwapChain()->GetBuffer(bufferIndex, IID_PPV_ARGS(&bufferList.at(bufferIndex))));

			// getting width and height out of gotten resource
			{
				D3D12_RESOURCE_DESC renderTargetDesc = bufferList.front()->GetDesc();

				m_width = renderTargetDesc.Width;
				m_height = renderTargetDesc.Height;
			}

			m_swapChainBuffer = std::make_shared<SwapChainRenderTarget>(*this, renderTargetFormat, bufferList);
		}

		// intializing back buffers
		m_backBuffer = std::make_shared<BackBufferRenderTarget>(*this, renderTargetFormat);


		// initializing depth stencil view
		m_depthStencilView = std::make_shared<DepthStencilViewMultiResource>(*this);

		// initializing graphic fence for each frame buffer
		for (unsigned int bufferIndex = 0; bufferIndex < swapChainBufferCount; bufferIndex++)
			m_graphicFences.push_back(Fence(*this));
	}

	descriptorHeap.Initialize(*this);
	constantBufferHeap.Initialize(*this);
	bufferHeap.Initialize(*this);
	renderer.Initialize(*this);
	profiler.Initialize(*this);
}

Graphics::~Graphics()
{
	WaitForGPU();
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

void Graphics::BeginFrame(float deltaTime)
{
	m_currentBufferIndex = GetCurrentBufferIndexFromSwapchain();

	m_imguiManager->BeginFrame(*this);

	// For now since we don't have proper synchronization system
	// We need to push copy events during runtime
	renderer.GetPipeline().BeginRender(*this);

	profiler.SetBeginData(*this, renderer.GetPipeline().GetGraphicCommandList(), deltaTime);

	graphicsBufferAllocatorManager.Update(*this);
}

void Graphics::FinishFrame()
{
	START_CPU_EVENT(PIX_COLOR(0, 255, 255), "Present");

	PresentFrame(*this);

	END_CPU_EVENT();

	START_CPU_EVENT(PIX_COLOR(0, 255, 255), "Cleanup");

	CleanupResources();

	ResourceList::ClearUnusedResources(*this);

	END_CPU_EVENT();

	WaitForGPUIfNeeded();
}

void Graphics::Render(Scene& scene, float deltaTime)
{
	renderer.Draw(*this, scene, deltaTime);
}

void Graphics::PresentFrame(Graphics& graphics)
{
	HRESULT hr;

	Fence* pPreviousFrameFence = &m_graphicFences.at(GetPreviousBufferIndex());

	// forcing this frame on GPU side to wait till previous frame is presented
	if(pPreviousFrameFence->GetValue() != 0)
		THROW_INFO_ERROR(GetDeviceResources().GetCommandQueue()->Wait(pPreviousFrameFence->Get(), pPreviousFrameFence->GetValue()));

	THROW_ERROR_AT_GFX_INIT(GetDeviceResources().GetSwapChain()->Present(1, NULL));
}

void Graphics::WaitForGPU()
{
	m_graphicFences.at(GetCurrentBufferIndex()).WaitForGPU(*this);
}

void Graphics::WaitForGPUIfNeeded()
{
	m_graphicFences.at(GetCurrentBufferIndex()).SetWaitValue(*this);

	m_graphicFences.at(GetNextBufferIndex()).WaitForValue(*this);
}

void Graphics::FinishInitialization()
{
	//initializing imgui
	m_imguiManager = std::make_unique<ImguiManager>(*this, m_windowHwnd);

	graphicsBufferAllocatorManager.Update(*this);
}

void Graphics::CleanupResources()
{
	resourceDeleter.Update(*this);
}

Fence* Graphics::GetFence(unsigned int frameIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to access Fence out of bounds", frameIndex > GetBufferCount() - 1);

	return &m_graphicFences.at(frameIndex);
}

GraphicsBufferAllocatorManager* Graphics::GetGraphicsBufferAllocatorManager()
{
	return &graphicsBufferAllocatorManager;
}

Profiler& Graphics::GetProfiler()
{
	return profiler;
}

Renderer& Graphics::GetRenderer()
{
	return renderer;
}

DeviceResources& Graphics::GetDeviceResources()
{
	return deviceResources;
}

ConstantBufferHeap& Graphics::GetConstantBufferHeap()
{
	return constantBufferHeap;
}

BufferHeap& Graphics::GetBufferHeap()
{
	return bufferHeap;
}

DescriptorHeap& Graphics::GetDescriptorHeap()
{
	return descriptorHeap;
}

FrameResourceDeleter* Graphics::GetFrameResourceDeleter()
{
	return &resourceDeleter;
}

ImguiManager* Graphics::GetImguiManager()
{
	return m_imguiManager.get();
}

#ifdef _DEBUG
InfoQueue* Graphics::GetInfoQueue()
{
	return m_infoQueue.get();
}
#endif

std::shared_ptr<SwapChainRenderTarget> Graphics::GetSwapChainBuffer()
{
	return m_swapChainBuffer;
}

std::shared_ptr<BackBufferRenderTarget> Graphics::GetBackBuffer()
{
	return m_backBuffer;
}

std::shared_ptr<DepthStencilViewMultiResource> Graphics::GetDepthStencil()
{
	return m_depthStencilView;
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

	GetDeviceResources().GetSwapChain()->QueryInterface(swapChain3.GetAddressOf());

	return swapChain3->GetCurrentBackBufferIndex();
}