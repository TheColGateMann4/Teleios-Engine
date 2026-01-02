#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

class Graphics;
class CommandList;
class ImguiManager;

class ImguiLayer
{
public:
	void Initialize(Graphics& graphics);

	void Render();

	void Draw(Graphics& graphics, CommandList* commandList);

	void ToggleCaptureInput(bool captureInput);

	void ToggleVisibility(bool newVisibility);

	void ToggleDemoVisibility(bool newDemoVisibility);

	void DrawDemoWindow();

	bool IsDemoVisible() const;

	bool IsVisible() const;

private:
	void GetImguiCommands(Graphics& graphics, ID3D12GraphicsCommandList* pDirectCommandList);

private:
	ImguiManager* m_imguiManager = nullptr;
	bool m_isVisible = true;
	bool m_isDemoVisible = false;
};