#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#include "Graphics/Core/Fence.h"
#include "Error/InfoQueue.h"
#include "RootSignature.h"
#include "Graphics/Bindables/RenderTarget.h"
#include "Graphics/Bindables/DepthStencilView.h"
#include "Graphics/Imgui/ImguiManager.h"
#include "FrameResourceDeleter.h"
#include "Graphics/Core/DescriptorHeap.h"
#include "ConstantBufferHeap.h"
#include "Graphics/Core/DeviceResources.h"

class Graphics
{
public:
	Graphics(HWND hWnd, DXGI_FORMAT renderTargetFormat);

	Graphics(const Graphics&) = delete;
	
	~Graphics();

public:
	unsigned int GetCurrentBufferIndex() const;
	unsigned int GetPreviousBufferIndex() const;
	unsigned int GetNextBufferIndex() const;
	unsigned int GetBufferCount() const;

	void BeginFrame();
	void FinishFrame();

	void WaitForGPU();
	void WaitForGPUIfNeeded(); // sets fence value for frame pushed to gpu, and waits for next buffer to be free to start drawing

private:
	void PresentFrame();
	void CleanupResources();

public:
	DeviceResources& GetDeviceResources();
	ConstantBufferHeap& GetConstantBufferHeap();
	DescriptorHeap& GetDescriptorHeap();
	FrameResourceDeleter* GetFrameResourceDeleter();
	ImguiManager* GetImguiManager();
	SwapChainRenderTarget* GetSwapChainBuffer();
	BackBufferRenderTarget* GetBackBuffer();
	DepthStencilViewMultiResource* GetDepthStencil();

#ifdef _DEBUG
	InfoQueue* GetInfoQueue();
#endif

	unsigned int GetWidth() const noexcept;
	unsigned int GetHeight() const noexcept;

private:
	static constexpr bool CheckValidRenderTargetFormat(DXGI_FORMAT format);
	unsigned int GetCurrentBufferIndexFromSwapchain();

private:
	DeviceResources deviceResources;
	ConstantBufferHeap constantBufferHeap;
	DescriptorHeap descriptorHeap;
	FrameResourceDeleter resourceDeleter;

private:
#ifdef _DEBUG
	std::unique_ptr<InfoQueue> m_infoQueue;
#endif	
	std::unique_ptr<ImguiManager> m_imguiManager;
	std::vector<Fence> m_graphicFences;

	std::shared_ptr<SwapChainRenderTarget> m_swapChainBuffer;
	std::shared_ptr<BackBufferRenderTarget> m_backBuffer;
	std::shared_ptr<DepthStencilViewMultiResource> m_depthStencilView;

private:
	unsigned int m_width = 0;
	unsigned int m_height = 0;

	const unsigned int swapChainBufferCount = 2;
	unsigned int m_currentBufferIndex = 0;
};