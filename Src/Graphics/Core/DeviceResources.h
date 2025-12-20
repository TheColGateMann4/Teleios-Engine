#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

class Graphics;

class DeviceResources
{
public:
	void InitializeEntryResources();
	void InitializeGraphicsResources(Graphics& graphics, HWND hWnd, DXGI_FORMAT format, unsigned int bufferCount);

public:
	IDXGIFactory2* GetFactory();
	ID3D12Debug6* GetDebugController();
	ID3D12Device10* GetDevice();
	ID3D12CommandQueue* GetCommandQueue();
	IDXGISwapChain* GetSwapChain();

private:
	Microsoft::WRL::ComPtr<IDXGIFactory2> pFactory;
	Microsoft::WRL::ComPtr<ID3D12Debug6> pDebugController;
	Microsoft::WRL::ComPtr<ID3D12Device10> pDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain;
};