#include <Windows.h>
#include <tchar.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <stdio.h>

#include "touchpad.h"

#include "termcolor.h"
#include "utils.h"

int mGetRawInputDeviceName(_In_ HANDLE hDevice, _Out_ TCHAR** deviceName, _Out_ UINT* nameSize, _Out_ unsigned int* cbDeviceName)
{
  int retval = 0;
  UINT winReturnCode;

  // validate pointer parameters
  if (deviceName == NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("(TCHAR**) deviceName is NULL!\n");
    printf(RESET_COLOR);
    exit(-1);
  }
  else if (nameSize == NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("nameSize is NULL! You will not able to know the number of characters in deviceName string.\n");
    printf(RESET_COLOR);
    exit(-1);
  }
  else if ((*deviceName) != NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("(TCHAR*) deviceName is not NULL! Please free your memory and set the pointer value to NULL.\n");
    printf(RESET_COLOR);
    exit(-1);
  }
  else if (cbDeviceName == NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("cbDeviceName is NULL! You will not able to know the size of the return array.\n");
    printf(RESET_COLOR);
    exit(-1);
  }
  else
  {
    // the actual code is here
    winReturnCode = GetRawInputDeviceInfo(_In_ hDevice, RIDI_DEVICENAME, NULL, nameSize);
    if (winReturnCode == (UINT)-1)
    {
      retval = -1;
      printf(FG_RED);
      printf("GetRawInputDeviceInfo failed at %s:%d\n", __FILE__, __LINE__);
      printf(RESET_COLOR);
      mGetLastError();
      exit(-1);
    }
    else
    {
      (*cbDeviceName) = (unsigned int)(sizeof(TCHAR) * ((*nameSize) + 1));
      (*deviceName)   = (TCHAR*)mMalloc((*cbDeviceName), __FILE__, __LINE__);

      (*deviceName)[(*nameSize)] = 0;

      winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, (*deviceName), nameSize);
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        printf(FG_RED);
        printf("GetRawInputDeviceInfo failed at %s:%d\n", __FILE__, __LINE__);
        printf(RESET_COLOR);
        exit(-1);
      }
      else if (winReturnCode != (*nameSize))
      {
        retval = -1;
        printf(FG_RED);
        printf("GetRawInputDeviceInfo does not return the expected size %d (actual) vs %d (expected) at  %s:%d\n", winReturnCode, (*nameSize), __FILE__, __LINE__);
        printf(RESET_COLOR);
        exit(-1);
      }
    }
  }

  return retval;
}

int mGetRawInputDeviceList(_Out_ UINT* numDevices, _Out_ RAWINPUTDEVICELIST** deviceList)
{
  int retval = 0;
  UINT winReturnCode;

  if (numDevices == NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("(UINT*) numDevices is NULL!\n");
    printf(RESET_COLOR);
    exit(-1);
  }
  else if (deviceList == NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("(RAWINPUTDEVICELIST**) deviceList is NULL!\n");
    printf(RESET_COLOR);
    exit(-1);
  }
  else if ((*deviceList) != NULL)
  {
    printf(FG_RED);
    printf("(RAWINPUTDEVICELIST*) deviceList is not NULL! Please free your memory and set the pointer to NULL.\n");
    printf(RESET_COLOR);
    exit(-1);
  }
  else
  {
    winReturnCode = GetRawInputDeviceList(NULL, numDevices, sizeof(RAWINPUTDEVICELIST));
    if (winReturnCode == (UINT)-1)
    {
      retval = -1;
      printf(FG_RED);
      printf("GetRawInputDeviceList failed at %s:%d\n", __FILE__, __LINE__);
      printf(RESET_COLOR);
      mGetLastError();
      exit(-1);
    }
    else
    {
      (*deviceList) = (RAWINPUTDEVICELIST*)mMalloc(sizeof(RAWINPUTDEVICELIST) * (*numDevices), __FILE__, __LINE__);
      winReturnCode = GetRawInputDeviceList((*deviceList), numDevices, sizeof(RAWINPUTDEVICELIST));
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        printf(FG_RED);
        printf("GetRawInputDeviceList failed at %s:%d\n", __FILE__, __LINE__);
        printf(RESET_COLOR);
        mGetLastError();
        // TODO should we also free (*deviceList) here?
        exit(-1);
      }
    }
  }

  return retval;
}

int mGetRawInputDevicePreparsedData(_In_ HANDLE hDevice, _Out_ PHIDP_PREPARSED_DATA* data, _Out_ UINT* cbSize)
{
  int retval = 0;
  UINT winReturnCode;

  if (data == NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("(PHIDP_PREPARSED_DATA*) data parameter is NULL!\n");
    printf(RESET_COLOR);

    exit(-1);
  }
  else if (cbSize == NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("The cbSize parameter is NULL!\n");
    printf(RESET_COLOR);

    exit(-1);
  }
  else if ((*data) != NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("(PHIDP_PREPARSED_DATA) data parameter is not NULL! Please free your memory and set the point to NULL.\n");
    printf(RESET_COLOR);

    exit(-1);
  }
  else
  {
    winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, NULL, cbSize);
    if (winReturnCode == (UINT)-1)
    {
      retval = -1;
      printf(FG_RED);
      printf("GetRawInputDeviceInfo failed at %s:%d\n", __FILE__, __LINE__);
      printf(RESET_COLOR);
      mGetLastError();

      exit(-1);
    }
    else
    {
      (*data) = (PHIDP_PREPARSED_DATA)mMalloc((*cbSize), __FILE__, __LINE__);

      winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, (*data), cbSize);
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        printf(FG_RED);
        printf("GetRawInputDeviceInfo failed at %s:%d\n", __FILE__, __LINE__);
        printf(RESET_COLOR);
        mGetLastError();

        exit(-1);
      }
    }
  }

  return retval;
}

int mGetRawInputData(_In_ HRAWINPUT hRawInput, _Out_ PUINT pcbSize, _Out_ LPVOID* pData)
{
  int retval = 0;
  UINT winReturnCode;

  if (pcbSize == NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("pcbSize parameter is NULL!\n");
    printf(RESET_COLOR);

    exit(-1);
  }
  else if (pData == NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("(LPVOID*) pData parameter is NULL!\n");
    printf(RESET_COLOR);

    exit(-1);
  }
  else if ((*pData) != NULL)
  {
    retval = -1;
    printf(FG_RED);
    printf("(LPVOID) pData value is not NULL! Please free your memory and set the pointer value to NULL.\n");
    printf(RESET_COLOR);

    exit(-1);
  }
  else
  {
    winReturnCode = GetRawInputData(hRawInput, RID_INPUT, NULL, pcbSize, sizeof(RAWINPUTHEADER));
    if (winReturnCode == (UINT)-1)
    {
      retval = -1;
      printf(FG_RED);
      printf("GetRawInputData failed at %s:%d\n", __FILE__, __LINE__);
      printf(RESET_COLOR);
      mGetLastError();

      exit(-1);
    }
    else
    {
      (*pData) = (LPVOID)mMalloc((*pcbSize), __FILE__, __LINE__);

      winReturnCode = GetRawInputData(hRawInput, RID_INPUT, (*pData), pcbSize, sizeof(RAWINPUTHEADER));
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        printf(FG_RED);
        printf("GetRawInputData failed at %s:%d\n", __FILE__, __LINE__);
        printf(RESET_COLOR);
        mGetLastError();

        exit(-1);
      }
      else if (winReturnCode != (*pcbSize))
      {
        retval = -1;
        printf(FG_RED);
        printf("GetRawInputData failed at %s:%d.\n", __FILE__, __LINE__);
        printf("The return value - the number of byte(s) copied into pData (%d) is not equal the expected value (%d).\n", winReturnCode, (*pcbSize));
        printf(RESET_COLOR);
        exit(-1);
      }
    }
  }

  return retval;
}
