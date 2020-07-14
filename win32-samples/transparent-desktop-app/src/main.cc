#include <Windows.h>

#include <d2d1.h>
#pragma comment(lib, "d2d1")

#include <tchar.h>
#include <iostream>

static TCHAR s_WindowClassName[]                         = _T("DesktopApp");
static TCHAR s_Title[]                                   = _T("Transparent Desktop App");
static ID2D1HwndRenderTarget* s_Direct2DHwndRenderTarget = NULL;
static ID2D1Factory* s_Direct2DFactory                   = NULL;
static ID2D1SolidColorBrush* s_Direct2DSolidColorBrush   = NULL;
static D2D1_ELLIPSE s_Direct2DEllipse;

void CalculateLayout() {
  if (s_Direct2DHwndRenderTarget) {
    D2D1_SIZE_F size    = s_Direct2DHwndRenderTarget->GetSize();
    const float radiusX = size.width / 2;
    const float radiusY = size.height / 2;
    s_Direct2DEllipse   = D2D1::Ellipse(D2D1::Point2F(radiusX, radiusY), radiusX, radiusY);
  }
}

LRESULT WINAPI WndProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
  HRESULT hr = S_OK;
  switch (uMsg) {
    case WM_CREATE: {
      hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &s_Direct2DFactory);
      if (FAILED(hr)) {
        std::cout << "D2D1CreateFactory failed!" << std::endl;
        return -1;
      }
    }
    case WM_PAINT: {
      if (s_Direct2DFactory == NULL) {
        std::cout << "s_Direct2DFactory is not initialized!" << std::endl;
        return -1;
      } else {
        if (s_Direct2DHwndRenderTarget == NULL) {
          RECT rc;
          GetClientRect(hWnd, &rc);

          D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

          hr = s_Direct2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size), &s_Direct2DHwndRenderTarget);

          if (SUCCEEDED(hr)) {
            const D2D1_COLOR_F color = D2D1::ColorF(0, 1.0f, 0, 0.5f);
            hr                       = s_Direct2DHwndRenderTarget->CreateSolidColorBrush(color, &s_Direct2DSolidColorBrush);

            if (SUCCEEDED(hr)) {
              CalculateLayout();
            } else {
              std::cout << "CreateSolidColorBrush failed!" << std::endl;
            }
          } else {
            std::cout << "CreateHwndRenderTarget failed!" << std::endl;
          }
        } else {
          PAINTSTRUCT ps;
          BeginPaint(hWnd, &ps);
          s_Direct2DHwndRenderTarget->BeginDraw();
          s_Direct2DHwndRenderTarget->Clear();
          s_Direct2DHwndRenderTarget->FillEllipse(s_Direct2DEllipse, s_Direct2DSolidColorBrush);

          hr = s_Direct2DHwndRenderTarget->EndDraw();

          if (FAILED(hr) || (hr == D2DERR_RECREATE_TARGET)) {
            if (s_Direct2DHwndRenderTarget != NULL) {
              s_Direct2DHwndRenderTarget->Release();
              s_Direct2DHwndRenderTarget = NULL;
            }

            if (s_Direct2DSolidColorBrush != NULL) {
              s_Direct2DSolidColorBrush->Release();
              s_Direct2DSolidColorBrush = NULL;
            }
          }

          EndPaint(hWnd, &ps);
        }
      }
      // if (s_Direct2DFactory == )
      break;
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      break;
    }
    default: {
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
  }

  return 0;
}

int mGetLastErrorMessage(DWORD errorCode) {
  // reference: https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessage

  DWORD dwFlags      = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;
  LPCVOID lpSource   = NULL;
  DWORD dwMessageId  = errorCode;
  DWORD dwLanguageId = 0;
  LPTSTR lpBuffer    = NULL;
  DWORD nSize        = 0;
  va_list* Arguments = NULL;

  size_t errorMessageSize = FormatMessage(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);

  if (errorMessageSize == 0) {
    // We will not call FormatMessage for this error because it may become infinite loop if we are not using the API correct.
    DWORD lastErrorCode = GetLastError();
    std::cout << "FormatMessage failed at error code " << lastErrorCode << std::endl;
    return -1;
  }

  std::wcout << lpBuffer << std::endl;
  LocalFree(lpBuffer);

  return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
  DWORD errorCode;

  std::cout << "COLOR_WINDOW: " << COLOR_WINDOW << std::endl;

  WNDCLASSEX wcex;

  // If we don't initialize every fields in the WNDCLASSEX struct, there will be memory access violation errors.
  wcex.cbSize        = sizeof(WNDCLASSEX);
  wcex.style         = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc   = WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = hInstance;
  wcex.hIcon         = LoadIcon(hInstance, IDI_APPLICATION);
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)0;
  wcex.lpszMenuName  = NULL;
  wcex.lpszClassName = s_WindowClassName;
  wcex.hIconSm       = LoadIcon(wcex.hInstance, IDI_APPLICATION);

  ATOM rce_ret = RegisterClassEx(&wcex);
  if (rce_ret == 0) {
    // RegisterClassEx failed
    errorCode = GetLastError();
    std::cout << "RegisterClassEx failed with error code " << errorCode << std::endl;
    mGetLastErrorMessage(errorCode);
    return -1;
  }

  // TODO Do we have to store the HINSTANCE in global variable? The sample code generated by Visual Studio stores it.

  // TODO Do we have to change any flags to make the window transparent?
  HWND hWnd = CreateWindow(s_WindowClassName, s_Title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 320, 320, NULL, NULL, hInstance, NULL);

  if (hWnd == NULL) {
    errorCode = GetLastError();
    std::cout << "CreateWindow failed with error code " << errorCode << std::endl;
    mGetLastErrorMessage(errorCode);
    return -1;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

int main() {
  return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
}
