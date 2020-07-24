#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <time.h>
#include <iostream>
#include <vector>

// the main window class name
static TCHAR szWindowClass[] = _T("DesktopApp");
// the application's title bar
static TCHAR szTitle[] = _T("GDI samples");

void mHandlePaintMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
  HDC hdc;
  HPEN pen;

  hdc = GetDC(hwnd);
  pen = CreatePen(PS_SOLID, 5, RGB(0, 255, 0));
  SelectObject(hdc, pen);

  MoveToEx(hdc, 20, 20, NULL);
  LineTo(hdc, 20, 200);
  LineTo(hdc, 200, 200);

  ReleaseDC(hwnd, hdc);
}

LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
  switch (uMsg) {
    case WM_PAINT: {
      mHandlePaintMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      break;
    }
    default: {
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
  }

  return 0;
}

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
  int nWidth  = 720;
  int nHeight = 480;

  WNDCLASSEX wcex;

  wcex.cbSize        = sizeof(WNDCLASSEX);
  wcex.style         = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc   = WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = hInstance;
  wcex.hIcon         = LoadIcon(hInstance, IDI_APPLICATION);
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(0);
  wcex.lpszMenuName  = NULL;
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm       = LoadIcon(wcex.hInstance, IDI_APPLICATION);

  if (!RegisterClassEx(&wcex)) {
    std::cout << "RegisterClassEx filed at " << __FILE__ << ":" << __LINE__ << std::endl;
    return -1;
  }

  HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_EX_LAYERED, CW_USEDEFAULT, CW_USEDEFAULT, nWidth, nHeight, NULL, NULL, hInstance, NULL);

  if (!hwnd) {
    std::cout << "CreateWindow filed at " << __FILE__ << ":" << __LINE__ << std::endl;
    return -1;
  }

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

int main() {
  return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
};
