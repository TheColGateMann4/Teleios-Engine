#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

#include "Fence.h"
#include "InfoQueue.h"
#include "RootSignature.h"
#include "RenderTarget.h"
#include "DepthStencilView.h"
#include "ImguiManager.h"
#include "FrameResourceDeleter.h"
#include "DescriptorHeap.h"

#include <dxgi1_6.h>

class Graphics
{
public:
	Graphics() = default;

	Graphics(const Graphics&) = delete;

public:
	void Initialize(HWND hWnd, DXGI_FORMAT renderTargetFormat);

public:
	unsigned int GetCurrentBufferIndex() const;
	unsigned int GetPreviousBufferIndex() const;
	unsigned int GetNextBufferIndex() const;
	unsigned int GetBufferCount() const;

	void BeginFrame();
	void FinishFrame();

	void WaitForGPU();

	void WaitForGPUIfNeeded(); // sets fence value for frame pushed to gpu, and waits for next buffer to be free to start drawing

	void CleanupResources();

public:
	DescriptorHeap& GetDescriptorHeap();
	FrameResourceDeleter* GetFrameResourceDeleter();
	ImguiManager* GetImguiManager();
	ID3D12Device* GetDevice();
	ID3D12CommandQueue* GetCommandQueue();

	BackBufferRenderTarget* GetBackBuffer();
	DepthStencilView* GetDepthStencil();

#ifdef _DEBUG
	InfoQueue* GetInfoQueue();
#endif

	unsigned int GetWidth() const noexcept;
	unsigned int GetHeight() const noexcept;

private:
	static constexpr bool CheckValidRenderTargetFormat(DXGI_FORMAT format);
	unsigned int GetCurrentBufferIndexFromSwapchain();

private:
	Microsoft::WRL::ComPtr<IDXGIFactory2> pFactory;
	Microsoft::WRL::ComPtr<ID3D12Debug6> pDebugController;
	Microsoft::WRL::ComPtr<ID3D12Device10> pDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain;

	DescriptorHeap descriptorHeap;
	FrameResourceDeleter resourceDeleter;

private:
#ifdef _DEBUG
	std::unique_ptr<InfoQueue> m_infoQueue;
#endif	
	std::unique_ptr<ImguiManager> m_imguiManager;
	std::vector<Fence> m_graphicFences;

	std::shared_ptr<BackBufferRenderTarget> m_backBuffer;
	std::shared_ptr<DepthStencilView> m_depthStencilView;

private:
	unsigned int m_width = 0;
	unsigned int m_height = 0;

	const unsigned int swapChainBufferCount = 10;
	unsigned int m_currentBufferIndex = 0;
};