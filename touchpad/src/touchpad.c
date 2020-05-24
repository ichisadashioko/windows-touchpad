// Following guide: https://docs.microsoft.com/en-us/cpp/windows/walkthrough-creating-windows-desktop-applications-cpp?view=vs-2019
#pragma once
#include "touchpad.h"

#include <Windows.h>
#include <hidusage.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

// Global variables

HINSTANCE hInst; // current instance
BOOL isTouchpadRegistered = FALSE;

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Windows Desktop Guided Tour Application");

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

    switch (message) {
        case WM_CREATE: {
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
                    debugf("Failed to register touchpad!\n");
                } else {
                    printTimestamp();
                    debugf("Successfully register touchpad!\n");
                    isTouchpadRegistered = TRUE;
                }
            }
        } break;
        case WM_INPUT: {
            if (isTouchpadRegistered) {
                // TODO Get and Parse HID input
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
    debugf("[%04d-%02d-%02d-%02d-%02d-%02d.%03d] ", ts.wYear, ts.wMonth, ts.wDay, ts.wHour, ts.wMinute, ts.wSecond, ts.wMilliseconds);
}

void debugf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    FILE *fp = fopen(DEBUG_FILE, "a+");

    if (fp != NULL) {
        vfprintf(fp, fmt, args);
        fclose(fp);
    }

    vfprintf(stdout, fmt, args);
    va_end(args);
}