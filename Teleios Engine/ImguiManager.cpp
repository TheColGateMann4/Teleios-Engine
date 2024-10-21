#include "ImguiManager.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

#include <imgui.h>
#include <backend/imgui_impl_dx12.h>
#include <backend/imgui_impl_win32.h>

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


	// descriptor heap initialization
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		THROW_ERROR(graphics.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pDescriptorHeap)));

		ImGui_ImplWin32_Init(hWnd);

		ImGui_ImplDX12_Init(
			graphics.GetDevice(),
			1,
			graphics.GetRenderTargetFormat(),
			pDescriptorHeap.Get(),
			pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			pDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
		);
	}
}

ImguiManager::~ImguiManager()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImguiManager::BeginFrame()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImguiManager::Render()
{
	ImGui::Render();
}

void ImguiManager::GetImguiCommands(Graphics& graphics, ID3D12GraphicsCommandList* pCommandList)
{
	THROW_INFO_ERROR(pCommandList->SetDescriptorHeaps(1, pDescriptorHeap.GetAddressOf()));

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT ImguiManager::HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
}