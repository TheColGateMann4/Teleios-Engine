#include "Window.h"
#include "Macros/ErrorMacros.h"
#include "Macros/KeyMacros.h"
//#include "resource.h" 

/*
		Window
*/

Window::Window(UINT32 width, UINT32 height, const char* name)
	:
	m_width(width),
	m_height(height),
	m_windowClass(name)
{
	// Window initialization
	{
		RECT rWindow = { NULL };

		rWindow.left = 100;
		rWindow.right = width + rWindow.left;

		rWindow.top = 100;
		rWindow.bottom = height + rWindow.top;

		DWORD creationFlags = WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;

		if (AdjustWindowRect(&rWindow, creationFlags, FALSE) == 0)
			THROW_LAST_ERROR;

		this->m_hWnd = CreateWindowA(
			m_windowClass.GetName(),
			name,
			creationFlags,
			CW_USEDEFAULT, CW_USEDEFAULT, // position X-Y 
			rWindow.right - rWindow.left, rWindow.bottom - rWindow.top, //size width-height
			NULL,
			NULL,
			m_windowClass.GetInstance(),
			this
		);

		if (this->m_hWnd == NULL)
			THROW_LAST_ERROR;

		ShowWindow(m_hWnd, SW_SHOW);
	}

	graphics.Initialize(m_hWnd);
}

Window::~Window()
{
	DestroyWindow(m_hWnd);
}

HWND Window::GetHWnd() const noexcept
{
	return m_hWnd;
}

BOOL Window::HandleMessages()
{
	MSG msg;

	while (PeekMessageA(&msg, NULL, NULL, NULL, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return static_cast<BOOL>(msg.wParam);

		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return TRUE;
}

UINT32 Window::GetWidth() const
{
	return m_width;
}

UINT32 Window::GetHeight() const
{
	return m_height;
}

/*
		Window Class
*/

Window::WindowClass::WindowClass(const char* name) noexcept
	:
	m_name(name),
	m_hInstance(GetModuleHandle(NULL))
{
	WNDCLASSEXA windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEXA);
	windowClass.style = CS_OWNDC;
	windowClass.lpfnWndProc = ::Window::HandleStartMessage;
	windowClass.cbClsExtra = NULL;
	windowClass.cbWndExtra = NULL;
	windowClass.hInstance = m_hInstance;
	windowClass.hIcon = NULL;
	//windowClass.hIcon = static_cast<HICON>(LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 96, 96, NULL));
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = GetName();
	windowClass.hIconSm = NULL;
	//windowClass.hIconSm = static_cast<HICON>(LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 96, 96, NULL));;

	RegisterClassExA(&windowClass);
}

Window::WindowClass::~WindowClass()
{
	UnregisterClassA(m_name.c_str(), GetInstance());
}

const char* Window::WindowClass::GetName() noexcept
{
	return m_name.c_str();
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return m_hInstance;
}

/*
		Functions fed to winapi that will be called from there
*/

LRESULT WINAPI Window::HandleStartMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (msg == WM_CREATE)
	{
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWindow = static_cast<Window*>(pCreate->lpCreateParams);

		SetWindowLongPtrA(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));

		SetWindowLongPtrA(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&MessageHub));

		return pWindow->HandleMessage(hWnd, msg, wParam, lParam);
	}

	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

LRESULT WINAPI Window::MessageHub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	Window* pWindow = reinterpret_cast<Window*>(GetWindowLongPtrA(hWnd, GWLP_USERDATA));

	return pWindow->HandleMessage(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	switch (msg)
	{
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_minimized = true;
				break;
			}
			else if (wParam == SIZE_RESTORED && m_minimized || wParam == SIZE_MAXIMIZED && m_minimized)
			{
				m_minimized = false;
				break;
			}

			m_width = static_cast<signed short>(lParam);
			m_height = lParam >> 16;

			break;
		}
	}

	return DefWindowProcA(hWnd, msg, wParam, lParam);
}