// Following guide: https://docs.microsoft.com/en-us/cpp/windows/walkthrough-creating-windows-desktop-applications-cpp?view=vs-2019
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <hidusage.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

// Global variables:
HINSTANCE hInst; // current instance
BOOL isTouchpadRegistered            = FALSE;
static TCHAR szWindowClass[]         = _T("DesktopApp");                              // the main window class name
static TCHAR szTitle[]               = _T("Windows Desktop Guided Tour Application"); // the application's title bar
static BOOL showedConnectedTouchpads = FALSE;

// Forward declarations of functions included in this code module:
int main();
void printTimestamp();
void printLastError();
LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow);

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
  WNDCLASSEX wcex;

  wcex.cbSize        = sizeof(WNDCLASSEX);
  wcex.style         = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc   = WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = hInstance;
  wcex.hIcon         = LoadIcon(hInstance, IDI_APPLICATION);
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName  = NULL;
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm       = LoadIcon(wcex.hInstance, IDI_APPLICATION);

  if (!RegisterClassEx(&wcex)) {
    MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Windows Desktop Guided Tour"), NULL);
    return 1;
  }

  hInst = hInstance;

  // The parameters to CreateWindow expained:
  // szWindowClass: the name of the application
  // szTitle: the text that appears in the title bar
  // WS_OVERLAPPEDWINDOW: the type of window to create
  // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
  // 500, 100: initial size (width, length)
  // NULL: the parent of this window
  // NULL: this application does not have a menu bar
  // hInstance: the first parameter from WinMain
  // NULL: not used in this application
  HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 100, NULL, NULL, hInstance, NULL);

  if (!hWnd) {
    MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Windows Desktop Guided Tour"), NULL);
    return 1;
  }

  // The parameters to ShowWindow explained:
  // hWnd: the value returned from CreateWindow
  // nCmdShow: the fourth parameter from WinMain
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  // Main message loop:
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

// FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
// PURPPOSE: Processes messages for the main window.
//
// WM_PAINT   - Paint the main window
// WM_DESTROY - Post a quit message and return
LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam) {
  PAINTSTRUCT ps;
  HDC hdc;
  TCHAR greeting[] = _T("Hello, Windows desktop!");
  UINT winReturnCode;

  switch (message) {
    case WM_CREATE: {
      if (!showedConnectedTouchpads) {
        showedConnectedTouchpads = TRUE;

        std::cout << "Retrieving all HID devices..." << std::endl;

        // find number of connected devices

        UINT numDevices = 0;
        winReturnCode   = GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
        if (winReturnCode == (UINT)-1) {
          std::cout << "Failed to get number of HID devices at " << __FILE__ << ":" << __LINE__ << std::endl;
          printLastError();
        } else {
          std::cout << "Detect " << numDevices << " device(s)!" << std::endl;
          std::unique_ptr<RAWINPUTDEVICELIST[]> rawInputDeviceList(new RAWINPUTDEVICELIST[numDevices]);

          // as we have to pass a PUINT to GetRawInputDeviceList the value might be changed.
          const UINT recordedNumDevices = numDevices;
          winReturnCode                 = GetRawInputDeviceList(rawInputDeviceList.get(), &numDevices, sizeof(RAWINPUTDEVICELIST));

          if (winReturnCode == (UINT)-1) {
            std::cout << "Failed to call GetRawInputDeviceList at " << __FILE__ << ":" << __LINE__ << std::endl;
            printLastError();
          } else {
            std::cout << "numDevices: " << numDevices << std::endl;
            for (UINT i = 0; i < recordedNumDevices; i++) {
              RAWINPUTDEVICELIST rawInputDevice = rawInputDeviceList.get()[i];
              std::cout << i << " - dwType: " << rawInputDevice.dwType << " - hDevice:" << rawInputDevice.hDevice << std::endl;

              // check device type
              switch (rawInputDevice.dwType) {
                case RIM_TYPEMOUSE: {
                  std::cout << "  Device type: mouse" << std::endl;
                } break;
                case RIM_TYPEKEYBOARD: {
                  std::cout << "  Device type: keyboard" << std::endl;
                } break;
                case RIM_TYPEHID: {
                  std::cout << "  Device type: a HID device (not mouse or keyboard)" << std::endl;
                } break;
                default: {
                  std::cout << "  Unknown device type: " << rawInputDevice.dwType << std::endl;
                } break;
              }

              // GetRawInputDeviceInfo (RIDI_DEVICENAME)
              UINT deviceNameBufferSize = 0;
              winReturnCode             = GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_DEVICENAME, NULL, &deviceNameBufferSize);
              if (winReturnCode == (UINT)-1) {
                std::cout << "Failed to call GetRawInputDeviceInfo for getting device name size at " << __FILE__ << ":" << __LINE__ << std::endl;
                printLastError();
              } else {
                std::cout << "  deviceNameLength: " << deviceNameBufferSize << std::endl;
                const UINT recordedDeviceNameBufferSize = deviceNameBufferSize;

                std::unique_ptr<TCHAR[]> deviceNameBuffer(new TCHAR[recordedDeviceNameBufferSize + 1]);

                // set string terminator
                // if we don't do this, bad thing will happen
                // the runtime will give all sort of exceptions that does not point to our code
                deviceNameBuffer.get()[recordedDeviceNameBufferSize] = 0;

                winReturnCode = GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_DEVICENAME, deviceNameBuffer.get(), &deviceNameBufferSize);
                if (winReturnCode == (UINT)-1) {
                  printf("Failed to get device name at %s:%d\n", __FILE__, __LINE__);
                  printLastError();
                } else if (winReturnCode != recordedDeviceNameBufferSize) {
                  printf("GetRawInputDeviceInfo does not return the expected size %d (actual) vs %d (expected)\n at %s:%d\n", winReturnCode, recordedDeviceNameBufferSize, __FILE__, __LINE__);
                } else {
                  TCHAR *deviceName = deviceNameBuffer.get();
                  std::cout << "  device name: ";
                  std::wcout << deviceName << std::endl;
                }
              }
            }
          }
        }
      }

      // TODO RegisterRawInputDevices
      if (!isTouchpadRegistered) {
        // register Windows Precision Touchpad top-level HID collection
        RAWINPUTDEVICE rid;

        rid.usUsagePage = HID_USAGE_PAGE_DIGITIZER;
        rid.usUsage     = HID_USAGE_DIGITIZER_TOUCH_PAD;
        rid.dwFlags     = RIDEV_INPUTSINK;
        rid.hwndTarget  = hWnd;

        if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) {
          isTouchpadRegistered = FALSE;
          printTimestamp();
          printf("Failed to register touchpad at %s:%d!\n", __FILE__, __LINE__);
          printLastError();
        } else {
          printTimestamp();
          printf("Successfully register touchpad!\n");
          isTouchpadRegistered = TRUE;
        }
      }
    } break;
    case WM_INPUT: {
      if (isTouchpadRegistered) {
        // Following guide: https://docs.microsoft.com/en-us/windows/win32/inputdev/using-raw-input#performing-a-standard-read-of-raw-input

        // Got our HID input message. This is to filter RIM_TYPEMOUSE and RIM_TYPEKEYBOARD.
        // rawInputHeader.hDevice;

        // Get the size of RAWINPUT by calling GetRawInputData() with pData = NULL

        UINT rawInputSize = 0;
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &rawInputSize, sizeof(RAWINPUTHEADER)) == (UINT)-1) {
          // handle error
          printTimestamp();
          printf("Retrieving RAWINPUT size. GetRawInputData() failed at %s:%d!\n", __FILE__, __LINE__);
          printLastError();
        } else {
          // rawInputSize will be modified to 0 after calling GetRawInputData() the second time
          const UINT _rawInputSize = rawInputSize; // backup the raw input size for checking with return value from GetRawInputData()
          LPBYTE lpb               = new BYTE[rawInputSize];
          if (lpb == NULL) {
            std::cout << "Failed to allocate memory for RAWINPUT at " << __FILE__ << ":" << __LINE__ << std::endl;
            printLastError();
          } else {
            winReturnCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &rawInputSize, sizeof(RAWINPUTHEADER));

            if (winReturnCode == (UINT)-1) {
              // handle error
              printTimestamp();
              printf("Coping RAWINPUT data. GetRawInputData() failed at %s:%d!\n", __FILE__, __LINE__);
              printLastError();
            } else if (winReturnCode != _rawInputSize) {
              printTimestamp();
              printf("Coping RAWINPUT data. The RAWINPUT size does not match! %s:%d\n", __FILE__, __LINE__);
            } else {
              // Parse the RAWINPUT data.
              RAWINPUT *raw = (RAWINPUT *)lpb;
              if (raw->header.dwType == RIM_TYPEHID) {
                DWORD sizeHid = raw->data.hid.dwSizeHid;
                DWORD count   = raw->data.hid.dwCount;
                BYTE *rawData = raw->data.hid.bRawData;

                if (count != 0) {
                }
              }
            }
          }

          delete[] lpb;
        }
      }
    } break;
    case WM_PAINT: {
      hdc = BeginPaint(hWnd, &ps);

      // Here your application is laid out.
      // For this introduction, we just print out "Hello, Windows desktop!"
      // in the top left corner.
      TextOut(hdc, 5, 5, greeting, _tcslen(greeting));
      // End application-specific layout section.

      EndPaint(hWnd, &ps);
    } break;
    case WM_DESTROY: {
      PostQuitMessage(0);
    } break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

void printTimestamp() {
  SYSTEMTIME ts;
  GetSystemTime(&ts);
  printf("[%04d-%02d-%02d-%02d-%02d-%02d.%03d] ", ts.wYear, ts.wMonth, ts.wDay, ts.wHour, ts.wMinute, ts.wSecond, ts.wMilliseconds);
}

void printLastError() {
  DWORD errorCode     = GetLastError();
  LPSTR messageBuffer = nullptr;
  size_t size         = FormatMessageA(
      // clang-format off
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer,
        0,
        NULL
    );
  // clang-format on

  printf("Error code: %d. Error message: %s", errorCode, messageBuffer);
}

int main() {
  auto ulongSize = sizeof(ULONG);
  printf("ULONG size: %d\n", ulongSize);
  return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
};