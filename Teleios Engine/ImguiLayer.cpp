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

uint8_t ImguiLayer::GetBitNum(ImGuiConfigFlags_ flag)
{
	return uint8_t( std::floor(std::sqrt( float(flag) )) );		// since bit value is pow of 2 we can just take sqrt of the value. We take floor of it since sometimes one flag has multiple bits
}

void ImguiLayer::SetCaptureInput(bool captureInput)
{
	ImGuiIO& io = ImGui::GetIO();

	std::bitset<sizeof(io.ConfigFlags) * 8> bits = io.ConfigFlags;

	// bit num is number of bit affcted by given flag
	static const uint8_t mouseBitNum = GetBitNum(ImGuiConfigFlags_NoMouse);
	static const uint8_t keyboardBitNum = GetBitNum(ImGuiConfigFlags_NoKeyboard);

	// setting bits affected by flags
	bits[keyboardBitNum] = !captureInput;
	bits[mouseBitNum] = !captureInput;

	// setting bits to imgui flag
	io.ConfigFlags = bits.to_ulong();
}