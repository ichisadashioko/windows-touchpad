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

  printf("Error code: %d. Error message: %s%s%s", errorCode, FG_RED, messageBuffer, RESET_COLOR);
}

GET_NUM_DEVICES_RETURN GetNumberOfDevices() {
  GET_NUM_DEVICES_RETURN retval;
  UINT numDevices    = 0;
  UINT winReturnCode = GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));

  if (winReturnCode == (UINT)-1) {
    std::cout << FG_RED << "Failed to get number of HID devices at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
    printLastError();

    retval = {numDevices, winReturnCode};
  } else {
    retval = {numDevices, (UINT)0};
  }

  return retval;
}