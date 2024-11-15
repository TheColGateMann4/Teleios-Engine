#include "Window.h"
#include "Macros/ErrorMacros.h"
#include "Macros/KeyMacros.h"
//#include "resource.h" 
#include "hidusage.h"

/*
		Window
*/

Window::Window(UINT32 width, UINT32 height, const char* name, DXGI_FORMAT colorSpace)
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

	graphics.Initialize(m_hWnd, colorSpace);

	// regitering for raw input notifications
	{
		RAWINPUTDEVICE rawInputDevice = {};
		rawInputDevice.usUsagePage = HID_USAGE_PAGE_GENERIC;
		rawInputDevice.usUsage = HID_USAGE_GENERIC_MOUSE;
		rawInputDevice.dwFlags = NULL; // RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE // could make use of these two flags that make mouse ignore everything
		rawInputDevice.hwndTarget = m_hWnd;

		if (RegisterRawInputDevices(&rawInputDevice, 1, sizeof(rawInputDevice)) == NULL)
			THROW_LAST_ERROR;
	}
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

void Window::LockCursor(bool lock, bool updateStatus)
{
	if (updateStatus)
		m_lockCursor = lock;

	if (lock)
	{
		RECT cursorClipRect = {};

		::GetClientRect(m_hWnd, &cursorClipRect);
		::MapWindowPoints(m_hWnd, HWND_DESKTOP, reinterpret_cast<POINT*>(&cursorClipRect), 2);

		if (::ClipCursor(&cursorClipRect) == 0)
			THROW_LAST_ERROR;
	}
	else
	{
		if (::ClipCursor(nullptr) == 0)
			THROW_LAST_ERROR;
	}

}

void Window::ShowCursor(bool show, bool updateStatus)
{
	if(updateStatus)
		m_showCursor = show;

	if (show)
		while (::ShowCursor(show) < 0);
	else
		while (::ShowCursor(show) >= 0);
}

bool Window::GetCursorVisibility() const
{
	return m_showCursor;
}

bool Window::GetCursorLocked() const
{
	return m_lockCursor;
}

void Window::OnFocusGain()
{
	// if cursor was previously locked, we lock it back
	if (m_lockCursor)
		LockCursor(m_lockCursor);

	if (!m_showCursor)
		this->ShowCursor(m_showCursor); // using this-> so it definetly calls local function instead windows.h one
}

void Window::OnLFocusLose()
{
	if (!m_showCursor)
		this->ShowCursor(true, false);
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

LRESULT WINAPI Window::HandleStartMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
	if (graphics.GetImguiManager()->HandleMessages(hWnd, msg, wParam, lParam))
		return true;

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

			m_width = static_cast<signed int>(lParam);
			m_height = lParam >> 16;

			break;
		}

		case WM_ACTIVATE:
		{
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
				OnFocusGain();
			else if (wParam == WA_INACTIVE)
				OnLFocusLose();

			break;
		}

		case WM_KILLFOCUS:
		{
			input.ReleaseAllKeys();

			break;
		}

		/*
					Keyboard 
		*/ 

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			if (!(lParam & (0b1 << 30)))// 30th bit counting from 0 coresponds to previous key state
				input.PushKeyEvent(wParam, Input::KeyState::Pressed);

			break;
		}
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			input.PushKeyEvent(wParam, Input::KeyState::Released);

			break;
		}

		/*
					Mouse
		*/

		case WM_LBUTTONDOWN:
			input.PushKeyEvent(VK_LBUTTON, Input::KeyState::Pressed); break;	
		case WM_LBUTTONUP:
			input.PushKeyEvent(VK_LBUTTON, Input::KeyState::Released); break;

		case WM_MBUTTONDOWN:
			input.PushKeyEvent(VK_MBUTTON, Input::KeyState::Pressed); break;	
		case WM_MBUTTONUP:
			input.PushKeyEvent(VK_MBUTTON, Input::KeyState::Released); break;

		case WM_RBUTTONDOWN:
			input.PushKeyEvent(VK_RBUTTON, Input::KeyState::Pressed); break;
		case WM_RBUTTONUP:
			input.PushKeyEvent(VK_RBUTTON, Input::KeyState::Released); break;

		case WM_XBUTTONDOWN:
			input.PushKeyEvent((static_cast<unsigned int>(wParam >> 16) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2, Input::KeyState::Pressed); break;
		case WM_XBUTTONUP:
			input.PushKeyEvent((static_cast<unsigned int>(wParam >> 16) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2, Input::KeyState::Released); break;
		
		case WM_MOUSEMOVE:
		{
			POINTS mousePosition = {};
			mousePosition.x = static_cast<SHORT>(lParam); // low order represents X position
			mousePosition.y = static_cast<SHORT>(lParam >> 16); // high order represents Y position

			input.SetMousePosition(mousePosition);

			break;
		}

		case WM_INPUT:
		{
			HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(lParam);
			UINT bufferSize = 0;


			// getting size of input header
			GetRawInputData(
				hRawInput,
				RID_INPUT,
				nullptr,
				&bufferSize,
				sizeof(RAWINPUTHEADER)
			);

			void* pData = new char[bufferSize];


			// getting our data
			GetRawInputData(
				hRawInput,
				RID_INPUT,
				pData,
				&bufferSize,
				sizeof(RAWINPUTHEADER)
			);

			RAWINPUT* pRawInputHeader = static_cast<RAWINPUT*>(pData);


			// if its not raw input from mouse we ignore it
			if (pRawInputHeader->header.dwType != RIM_TYPEMOUSE)
			{
				delete[] pData;
				break;
			}

			POINTS mousePosition = { static_cast<SHORT>(pRawInputHeader->data.mouse.lLastX) , static_cast<SHORT>(pRawInputHeader->data.mouse.lLastY) };

			if(mousePosition.x != 0 || mousePosition.y != 0)
				input.SetMouseDelta(mousePosition);

			delete[] pData;
			break;
		}
	}

	return DefWindowProcA(hWnd, msg, wParam, lParam);
}