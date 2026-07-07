#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

class Graphics;

class ImguiManager
{
public:
	ImguiManager(Graphics& graphics, HWND hWnd);

	~ImguiManager();

public:
	void BeginFrame(Graphics& graphics);

	static LRESULT HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};