#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;
class Pipeline;
class ImguiManager;

class ImguiLayer
{
public:
	ImguiLayer(Graphics& graphics);

public:
	void Render();

	void Draw(Graphics& graphics, Pipeline& pipeline);

private:
	void GetImguiCommands(Graphics& graphics, ID3D12GraphicsCommandList* pDirectCommandList);

private:
	ImguiManager* m_imguiManager;
};