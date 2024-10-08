#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class ImguiManager
{
public:
	ImguiManager(Graphics& graphics, HWND hWnd);

	~ImguiManager();

public:
	void BeginFrame();

	void Render();

	void GetImguiCommands(Graphics& graphics, ID3D12GraphicsCommandList* pCommandList);

	LRESULT HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
};

