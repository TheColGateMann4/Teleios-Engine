#include "ImguiLayer.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "ImguiManager.h"

#include "Macros/ErrorMacros.h"

#include <imgui.h>
#include <backend/imgui_impl_dx12.h>
#include <backend/imgui_impl_win32.h>

#include <bitset>

ImguiLayer::ImguiLayer(Graphics& graphics)
	:
	m_imguiManager(graphics.GetImguiManager())
{

}

void ImguiLayer::Render()
{
	ImGui::Render();
}

void ImguiLayer::GetImguiCommands(Graphics& graphics, ID3D12GraphicsCommandList* pDirectCommandList)
{
	THROW_INFO_ERROR(pDirectCommandList->SetDescriptorHeaps(1, m_imguiManager->GetAddressOfDescriptorHeap()));

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pDirectCommandList);
}

void ImguiLayer::Draw(Graphics& graphics, Pipeline& pipeline)
{
	CommandList* directCommandList = pipeline.GetGraphicCommandList();

	directCommandList->SetRenderTarget(graphics, graphics.GetBackBuffer(), graphics.GetDepthStencil());

	directCommandList->SetResourceState(graphics, graphics.GetBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	GetImguiCommands(graphics, directCommandList->Get());

	directCommandList->SetResourceState(graphics, graphics.GetBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
}

void ImguiLayer::ToggleCaptureInput(bool captureInput)
{
	ImGuiIO& io = ImGui::GetIO();

	int flags = ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoKeyboard;

	if (captureInput)
		io.ConfigFlags &= ~flags;
	else
		io.ConfigFlags |= flags;
}