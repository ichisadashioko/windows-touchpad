#include <Windows.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <iostream>

#include "termcolor.h"
#include "utils.h"
#include "touchpad.h"

int mGetRawInputDeviceName(HANDLE hDevice, TCHAR** deviceName, UINT* nameSize, unsigned int* cbDeviceName) {
  int retval = 0;
  UINT winReturnCode;

  if (deviceName == NULL) {
    retval = -1;
    std::cout << FG_RED << "(TCHAR**) deviceName is NULL!" << RESET_COLOR << std::endl;
    throw;
    exit(-1);
  } else if (nameSize == NULL) {
    retval = -1;
    std::cout << FG_RED << "nameSize is NULL! You will not able to know the number of characters in deviceName string." << RESET_COLOR << std::endl;
    throw;
    exit(-1);
  } else if ((*deviceName) != NULL) {
    retval = -1;
    std::cout << FG_RED << "(TCHAR*) deviceName is not NULL! Please free your memory and set the pointer value to NULL." << RESET_COLOR << std::endl;
    throw;
    exit(-1);
  } else if (cbDeviceName == NULL) {
    retval = -1;
    std::cout << FG_RED << "cbDeviceName is NULL! You will not able to know the size of the return array." << RESET_COLOR << std::endl;
    throw;
    exit(-1);
  } else {
    winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, NULL, nameSize);
    if (winReturnCode == (UINT)-1) {
      retval = -1;
      std::cout << FG_RED << "GetRawInputDeviceInfo failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      mGetLastError();
      throw;
      exit(-1);
    } else {
      (*cbDeviceName) = (unsigned int)(sizeof(TCHAR) * ((*nameSize) + 1));
      (*deviceName)   = (TCHAR*)mMalloc((*cbDeviceName), __FILE__, __LINE__);

      (*deviceName)[(*nameSize)] = 0;

      winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, (*deviceName), nameSize);
      if (winReturnCode == (UINT)-1) {
        retval = -1;
        std::cout << FG_RED << "GetRawInputDeviceInfo failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
        mGetLastError();
        throw;
        exit(-1);
      } else if (winReturnCode != (*nameSize)) {
        retval = -1;
        std::cout << FG_RED << "GetRawInputDeviceInfo does not return the expected size " << winReturnCode << " (actual) vs " << (*nameSize) << " (expected) at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
        throw;
        exit(-1);
      }
    }
  }

  return retval;
}
