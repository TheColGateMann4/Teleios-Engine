#include "ImguiManager.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/Graphics.h"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

void SrvDescriptorAllocFn(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
{
	Graphics* pGraphics = static_cast<Graphics*>(info->UserData);
	
	DescriptorHeap::DescriptorInfo descriptorInfo = pGraphics->GetDescriptorHeap().GetNextHandle();
	
	*out_cpu_desc_handle = descriptorInfo.descriptorCpuHandle;
	*out_gpu_desc_handle = descriptorInfo.descriptorHeapGpuHandle;
}

void SrvDescriptorFreeFn(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle)
{
	// TODO: for now we don't handle free'ing descriptors
}

ImguiManager::ImguiManager(Graphics& graphics, HWND hWnd)
{
	HRESULT hr;

	// imgui initialization stuff
	{
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
	}

	// setting style for our imgui layer
	ImGui::StyleColorsDark();

	THROW_INTERNAL_ERROR_IF("Failed to initialize imgui win32 backend", !ImGui_ImplWin32_Init(hWnd));

	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = graphics.GetDeviceResources().GetDevice();
	initInfo.CommandQueue = graphics.GetDeviceResources().GetCommandQueue();
	initInfo.NumFramesInFlight = graphics.GetBufferCount();
	initInfo.RTVFormat = graphics.GetBackBuffer()->GetFormat();
	initInfo.DSVFormat = graphics.GetDepthStencil()->GetFormat();
	initInfo.UserData = &graphics;
	initInfo.SrvDescriptorHeap = graphics.GetDescriptorHeap().Get();
	initInfo.SrvDescriptorAllocFn = SrvDescriptorAllocFn;
	initInfo.SrvDescriptorFreeFn = SrvDescriptorFreeFn;

	THROW_INTERNAL_ERROR_IF("Failed to initialize imgui directx12 backend", !ImGui_ImplDX12_Init(&initInfo));
}

ImguiManager::~ImguiManager()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImguiManager::BeginFrame(Graphics& graphics)
{
#ifdef _DEBUG
	InfoQueue* infoQueue = graphics.GetInfoQueue();

	// muting warnings or messages that imgui will generate since it is not on time with DX12 previews
	infoQueue->SetMuteInfoMessages(true);
#endif

	{
		THROW_INFO_ERROR(ImGui_ImplDX12_NewFrame());
		THROW_INFO_ERROR(ImGui_ImplWin32_NewFrame());
		THROW_INFO_ERROR(ImGui::NewFrame());
	}

#ifdef _DEBUG
	infoQueue->SetMuteInfoMessages(false);
#endif
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT ImguiManager::HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}