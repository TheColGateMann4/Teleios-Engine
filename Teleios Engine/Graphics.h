#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "RenderTarget.h"
#include "InfoQueue.h"
#include <dxgi1_6.h>

class Graphics
{
public:
	void Initialize(HWND hWnd, DXGI_FORMAT colorSpace);

public:
	unsigned int GetCurrentBackBufferIndex();

	void FinishFrame();

public:
	ID3D12Device* GetDevice();

	BackBufferRenderTarget* GetBackBuffer();
	InfoQueue* GetInfoQueue();

	DXGI_FORMAT GetColorSpace() const noexcept;

private:
	static constexpr bool CheckValidColorSpace(DXGI_FORMAT format);

private:
	Microsoft::WRL::ComPtr<IDXGIFactory4> pFactory;
	Microsoft::WRL::ComPtr<ID3D12Debug> pDebugController;
	Microsoft::WRL::ComPtr<ID3D12Device> pDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeapDescriptor;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState;

private:
	std::shared_ptr<BackBufferRenderTarget> m_backBuffer;
	std::unique_ptr<InfoQueue> m_infoQueue;

private:
	DXGI_FORMAT m_colorSpace;
};