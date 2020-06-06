// clang-format off
#include <Windows.h>
#include <hidusage.h>
#include <hidpi.h>
// clang-format on
#include <iostream>
#include "termcolor.h"
#include "hid-utils.h"

void printTimestamp() {
    SYSTEMTIME ts;
    GetSystemTime(&ts);
    printf("%s[%04d-%02d-%02d-%02d-%02d-%02d.%03d]%s ", FG_GREEN, ts.wYear, ts.wMonth, ts.wDay, ts.wHour, ts.wMinute, ts.wSecond, ts.wMilliseconds, RESET_COLOR);
}

void printLastError() {
    DWORD errorCode     = GetLastError();
    LPSTR messageBuffer = nullptr;
    size_t size         = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    printf("Error code: %d. Error message: %s%s%s", errorCode, FG_RED, messageBuffer, RESET_COLOR);
}