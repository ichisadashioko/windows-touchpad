#include <Windows.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <iostream>

#include "termcolor.h"
#include "utils.h"
#include "touchpad.h"

int mGetRawInputDeviceName(_In_ HANDLE hDevice, _Out_ TCHAR** deviceName, _Out_ UINT* nameSize, _Out_ unsigned int* cbDeviceName) {
  int retval = 0;
  UINT winReturnCode;

  // validate pointer parameters
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
    // the actual code is here
    winReturnCode = GetRawInputDeviceInfo(_In_ hDevice, RIDI_DEVICENAME, NULL, nameSize);
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

int mGetRawInputDeviceList(_Out_ UINT* numDevices, _Out_ RAWINPUTDEVICELIST** deviceList) {
  int retval = 0;
  UINT winReturnCode;

  if (numDevices == NULL) {
    retval = -1;
    std::cout << FG_RED << "numDevices is NULL!" << RESET_COLOR << std::endl;
    throw;
    exit(-1);
  } else if (deviceList == NULL) {
    retval = -1;
    std::cout << FG_RED << "(RAWINPUTDEVICELIST**) deviceList is NULL!" << RESET_COLOR << std::endl;
    throw;
    exit(-1);
  } else if ((*deviceList) != NULL) {
    std::cout << FG_RED << "(RAWINPUTDEVICELIST*) deviceList is not NULL! Please free your memory and set the pointer to NULL." << RESET_COLOR << std::endl;
    throw;
    exit(-1);
  } else {
    winReturnCode = GetRawInputDeviceList(NULL, numDevices, sizeof(RAWINPUTDEVICELIST));
    if (winReturnCode == (UINT)-1) {
      retval = -1;
      std::cout << FG_RED << "GetRawInputDeviceList failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      mGetLastError();
      throw;
      exit(-1);
    } else {
      (*deviceList) = (RAWINPUTDEVICELIST*)mMalloc(sizeof(RAWINPUTDEVICELIST) * (*numDevices), __FILE__, __LINE__);
      winReturnCode = GetRawInputDeviceList((*deviceList), numDevices, sizeof(RAWINPUTDEVICELIST));
      if (winReturnCode == (UINT)-1) {
        retval = -1;
        std::cout << "GetRawInputDeviceList failed at " << __FILE__ << ":" << __LINE__ << std::endl;
        mGetLastError();
        // TODO should we also free (*deviceList) here?
        throw;
        exit(-1);
      }
    }
  }

  return retval;
}

int mGetRawInputDevicePreparsedData(_In_ HANDLE hDevice, _Out_ PHIDP_PREPARSED_DATA* data, _Out_ UINT* cbSize) {
  int retval = 0;
  UINT winReturnCode;

  if (data == NULL) {
    retval = -1;
    std::cout << FG_RED << "(PHIDP_PREPARSED_DATA*) data parameter is NULL!" << RESET_COLOR << std::endl;

    throw;
    exit(-1);
  } else if (cbSize == NULL) {
    retval = -1;
    std::cout << FG_RED << "The cbSize parameter is NULL!" << RESET_COLOR << std::endl;

    throw;
    exit(-1);
  } else if ((*data) != NULL) {
    retval = -1;
    std::cout << FG_RED << "(PHIDP_PREPARSED_DATA) data parameter is not NULL! Please free your memory and set the point to NULL." << RESET_COLOR << std::endl;

    throw;
    exit(-1);
  } else {
    winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, NULL, cbSize);
    if (winReturnCode == (UINT)-1) {
      retval = -1;
      std::cout << FG_RED << "GetRawInputDeviceInfo failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      mGetLastError();

      throw;
      exit(-1);
    } else {
      (*data) = (PHIDP_PREPARSED_DATA)mMalloc((*cbSize), __FILE__, __LINE__);

      winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, (*data), cbSize);
      if (winReturnCode == (UINT)-1) {
        retval = -1;
        std::cout << FG_RED << "GetRawInputDeviceInfo failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
        mGetLastError();

        throw;
        exit(-1);
      }
    }
  }

  return retval;
}

int mGetRawInputData(_In_ HRAWINPUT hRawInput, _Out_ PUINT pcbSize, _Out_ LPVOID* pData) {
  int retval = 0;
  UINT winReturnCode;

  if (pcbSize == NULL) {
    retval = -1;
    std::cout << FG_RED << "pcbSize parameter is NULL!" << RESET_COLOR << std::endl;

    throw;
    exit(-1);
  } else if (pData == NULL) {
    retval = -1;
    std::cout << FG_RED << "(LPVOID*) pData parameter is NULL!" << RESET_COLOR << std::endl;

    throw;
    exit(-1);
  } else if ((*pData) != NULL) {
    retval = -1;
    std::cout << FG_RED << "(LPVOID) pData value is not NULL! Please free your memory and set the pointer value to NULL." << RESET_COLOR << std::endl;

    throw;
    exit(-1);
  } else {
    winReturnCode = GetRawInputData(hRawInput, RID_INPUT, NULL, pcbSize, sizeof(RAWINPUTHEADER));
    if (winReturnCode == (UINT)-1) {
      retval = -1;
      std::cout << FG_RED << "GetRawInputData failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      mGetLastError();

      throw;
      exit(-1);
    } else {
      (*pData) = (LPVOID)mMalloc((*pcbSize), __FILE__, __LINE__);

      winReturnCode = GetRawInputData(hRawInput, RID_INPUT, (*pData), pcbSize, sizeof(RAWINPUTHEADER));
      if (winReturnCode == (UINT)-1) {
        retval = -1;
        std::cout << FG_RED << "GetRawInputData failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
        mGetLastError();

        throw;
        exit(-1);
      } else if (winReturnCode != (*pcbSize)) {
        retval = -1;
        std::cout << FG_RED << "GetRawInputData failed at " << __FILE__ << ":" << __LINE__ << ". The return value - the number of byte(s) copied into pData (" << winReturnCode << ") is not equal the expected value (" << (*pcbSize) << ")." << RESET_COLOR << std::endl;

        throw;
        exit(-1);
      }
    }
  }

  return retval;
}
