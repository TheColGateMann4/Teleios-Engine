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

	LRESULT HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	ID3D12DescriptorHeap* const* GetAddressOfDescriptorHeap() const;

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
};

