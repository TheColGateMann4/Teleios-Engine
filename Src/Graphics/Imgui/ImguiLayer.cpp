#include "ImguiLayer.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/CommandList.h"
#include "Graphics/Core/Pipeline.h"
#include "ImguiManager.h"

#include "Macros/ErrorMacros.h"

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

#include <bitset>

void ImguiLayer::Initialize(Graphics& graphics)
{
	m_imguiManager = graphics.GetImguiManager();
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

void ImguiLayer::Draw(Graphics& graphics, CommandList* commandList)
{
	GetImguiCommands(graphics, commandList->Get());
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


void ImguiLayer::ToggleVisibility(bool newVisibility)
{
	m_isVisible = newVisibility;
}

void ImguiLayer::ToggleDemoVisibility(bool newDemoVisibility)
{
	m_isDemoVisible = newDemoVisibility;
}

void ImguiLayer::DrawDemoWindow()
{
	if (!m_isVisible || !m_isDemoVisible)
		return;

	ImGui::ShowDemoWindow(&m_isDemoVisible);
}

bool ImguiLayer::IsDemoVisible() const
{
	return m_isDemoVisible;
}

bool ImguiLayer::IsVisible() const
{
	return m_isVisible;
}