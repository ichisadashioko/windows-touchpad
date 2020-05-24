// Following guide: https://docs.microsoft.com/en-us/cpp/windows/walkthrough-creating-windows-desktop-applications-cpp?view=vs-2019
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define DEBUG_FILE "debug-output.log"

#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

// Function signatures

void debugf(const char *fmt, ...);
void printTimestamp();

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow);
