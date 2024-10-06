#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

#include "InfoQueue.h"
#include "RootSignature.h"
#include "RenderTarget.h"
#include "DepthStencilView.h"

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
	ID3D12CommandQueue* GetCommandQueue();

	RootSignature* GetRootSignature();
	BackBufferRenderTarget* GetBackBuffer();
	DepthStencilView* GetDepthStencil();
	InfoQueue* GetInfoQueue();

	DXGI_FORMAT GetColorSpace() const noexcept;
	unsigned int GetWidth() const noexcept;
	unsigned int GetHeight() const noexcept;

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
	std::unique_ptr<InfoQueue> m_infoQueue;
	std::unique_ptr<RootSignature> m_rootSignature;
	std::shared_ptr<BackBufferRenderTarget> m_backBuffer;
	std::shared_ptr<DepthStencilView> m_depthStencilView;

private:
	DXGI_FORMAT m_colorSpace;
	unsigned int m_width;
	unsigned int m_height;
};