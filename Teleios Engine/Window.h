#pragma once
#include "Includes/CppIncludes.h"
#include "ErrorHandler.h"
#include "Graphics.h"
#include "Input.h"

class Window
{
public:
	Window(UINT32 width, UINT32 height, const char* name, DXGI_FORMAT colorSpace);
	~Window();
	Window(const Window&) = delete;
	Window& operator= (const Window&) = delete;

public:
	HWND GetHWnd() const noexcept;
	BOOL HandleMessages();

public:
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;

private:
	class WindowClass
	{
	public:
		WindowClass(const char* windowClassName) noexcept;
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator= (const WindowClass&) = delete;

	public:
		const char* GetName() noexcept;
		HINSTANCE GetInstance() noexcept;

	private:
		std::string m_name;
		HINSTANCE m_hInstance;
	};

private:
	static LRESULT WINAPI HandleStartMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT WINAPI MessageHub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

private:
	WindowClass m_windowClass;
	HWND m_hWnd;
	unsigned int m_width;
	unsigned int m_height;

	bool m_minimized = false;

public:
	Graphics graphics;
	Input input;
};