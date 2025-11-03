#include "platformWin32.h"
#include <stdexcept>

#ifndef UNICODE
#error "UNICODE must be defined"
#endif

jst::Window::Window(unsigned int width, unsigned int height)
    : width(width)
    , height(height) {
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

  HINSTANCE hInstance = GetModuleHandle(nullptr);
  WNDCLASSEXW wndClass{
    .cbSize = sizeof(WNDCLASSEX),
    .lpfnWndProc = WindowProc,
    .hInstance = hInstance,
    .hCursor = LoadCursor(nullptr, IDC_ARROW),
    .lpszClassName = L"MainWindowClass",
  };
  DWORD exStyle = WS_EX_OVERLAPPEDWINDOW;
  DWORD style = WS_OVERLAPPEDWINDOW;
  RECT rect{ 0, 0, (LONG)width, (LONG)height };
  AdjustWindowRectEx(&rect, style, FALSE, exStyle);
  RegisterClassEx(&wndClass);
  hwnd = CreateWindowEx(exStyle, L"MainWindowClass", L"MainWindow", style, 5, 5, rect.right - rect.left,
                         rect.bottom - rect.top, nullptr, nullptr, hInstance, this);
  if (hwnd == nullptr) {
    throw std::runtime_error("Window creation failed");
  }

  ShowWindow(hwnd, SW_SHOWDEFAULT);
}

LRESULT CALLBACK jst::Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_NCCREATE) {
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	Window* pThis = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	return pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
}

LRESULT jst::Window::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool jst::Window::ShouldQuit() const {
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT) {
			return true;
		}
	}
	return false;
}

void jst::Window::SetTitle(std::string const& title) {
	std::wstring wTitle(title.begin(), title.end());
	SetWindowText(hwnd, wTitle.c_str());
}
