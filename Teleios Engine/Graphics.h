#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "RenderTarget.h"
#include <dxgi1_6.h>

class Graphics
{
public:
	void Initialize(HWND hWnd);

public:
	unsigned int GetCurrentBackBufferIndex();

	void FinishFrame();

public:
	BackBufferRenderTarget* GetBackBuffer();

	DXGI_FORMAT GetColorSpace() const noexcept;

private:
	Microsoft::WRL::ComPtr<IDXGIFactory4> pFactory;
	Microsoft::WRL::ComPtr<ID3D12Debug> pDebugController;
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> pDebugInfoQueue;
	Microsoft::WRL::ComPtr<ID3D12Device> pDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeapDescriptor;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState;

private:
	std::shared_ptr<BackBufferRenderTarget> m_backBuffer;

private:
	DXGI_FORMAT m_colorSpace;
};