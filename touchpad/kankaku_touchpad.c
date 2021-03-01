#pragma once
#include "kankaku_touchpad.h"

#include <Windows.h>
#include <tchar.h>
#include <sal.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <stdio.h>

#include "termcolor.h"
#include "kankaku_utils.h"

int mGetRawInputDeviceName(_In_ HANDLE hDevice, _Out_ TCHAR** deviceName, _Out_ UINT* nameSize, _Out_ unsigned int* cbDeviceName)
{
  int retval = 0;
  UINT winReturnCode;

  // validate pointer parameters
  if (deviceName == NULL)
  {
    retval = -1;
    fprintf(stderr, "%s(TCHAR**) deviceName is NULL%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else if (nameSize == NULL)
  {
    retval = -1;
    fprintf(stderr, "%snameSize is NULL! You will not able to know the number of characters in deviceName string%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else if ((*deviceName) != NULL)
  {
    retval = -1;
    fprintf(stderr, "%s(TCHAR*) deviceName is not NULL! Please free your memory and set the pointer value to NULL%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else if (cbDeviceName == NULL)
  {
    retval = -1;
    fprintf(stderr, "%scbDeviceName is NULL! You will not able to know the size of the return array%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else
  {
    // the actual code is here
    winReturnCode = GetRawInputDeviceInfo(_In_ hDevice, RIDI_DEVICENAME, NULL, nameSize);
    if (winReturnCode == (UINT)-1)
    {
      retval = -1;
      fprintf(stderr, "%sGetRawInputDeviceInfo failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
      utils_print_win32_last_error();
      exit(-1);
    }
    else
    {
      (*cbDeviceName) = (unsigned int)(sizeof(TCHAR) * ((*nameSize) + 1));
      (*deviceName)   = (TCHAR*)kankaku_utils_malloc_or_die((*cbDeviceName), __FILE__, __LINE__);

      (*deviceName)[(*nameSize)] = 0;

      winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, (*deviceName), nameSize);
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        fprintf(stderr, "%sGetRawInputDeviceInfo failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
        utils_print_win32_last_error();
        exit(-1);
      }
      else if (winReturnCode != (*nameSize))
      {
        retval = -1;
        fprintf(stderr, "%sGetRawInputDeviceInfo does not return the expected size %d (actual) vs %d (expected) at  %s:%d%s\n", FG_BRIGHT_RED, winReturnCode, (*nameSize), __FILE__, __LINE__, RESET_COLOR);
        exit(-1);
      }
    }
  }

  return retval;
}

int kankaku_touchpad_get_raw_input_device_list(_Out_ UINT* numDevices, _Out_ RAWINPUTDEVICELIST** deviceList)
{
  int retval = 0;
  UINT winReturnCode;

#pragma region input checking
  if (numDevices == NULL)
  {
    retval = -1;
    fprintf(stderr, "%s(UINT*) numDevices is NULL%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else if (deviceList == NULL)
  {
    retval = -1;
    fprintf(stderr, "%s(RAWINPUTDEVICELIST**) deviceList is NULL%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else if ((*deviceList) != NULL)
  {
    retval = -1;
    fprintf(stderr, "%s(RAWINPUTDEVICELIST*) deviceList is not NULL! Please free your memory and set the pointer to NULL%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else
#pragma endregion end input checking
  {
    winReturnCode = GetRawInputDeviceList(NULL, numDevices, sizeof(RAWINPUTDEVICELIST));
    if (winReturnCode == (UINT)-1)
    {
      retval = -1;
      fprintf(stderr, "%sGetRawInputDeviceList failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
      utils_print_win32_last_error();
      exit(-1);
    }
    else
    {
      (*deviceList) = (RAWINPUTDEVICELIST*)kankaku_utils_malloc_or_die(sizeof(RAWINPUTDEVICELIST) * (*numDevices), __FILE__, __LINE__);
      winReturnCode = GetRawInputDeviceList((*deviceList), numDevices, sizeof(RAWINPUTDEVICELIST));
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        fprintf(stderr, "%sGetRawInputDeviceList failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
        utils_print_win32_last_error();
        // TODO should we also free (*deviceList) here?
        exit(-1);
      }
    }
  }

  return retval;
}

/*
note: example for allocating memory for pointer parameter
*/
int kankaku_touchpad_get_raw_input_device_preparsed_data(_In_ HANDLE hDevice, _Out_ PHIDP_PREPARSED_DATA* data, _Out_ UINT* cbSize)
{
  int retval = 0;
  UINT winReturnCode;

  if (data == NULL)
  {
    retval = -1;
    fprintf(stderr, "%s(PHIDP_PREPARSED_DATA*) data parameter is NULL%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else if (cbSize == NULL)
  {
    retval = -1;
    fprintf(stderr, "%sThe cbSize parameter is NULL%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else if ((*data) != NULL)
  {
    retval = -1;
    fprintf(stderr, "%s(PHIDP_PREPARSED_DATA) data parameter is not NULL! Please free your memory and set the point to NULL%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else
  {
    winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, NULL, cbSize);
    if (winReturnCode == (UINT)-1)
    {
      retval = -1;
      fprintf(stderr, "%sGetRawInputDeviceInfo failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
      utils_print_win32_last_error();
      exit(-1);
    }
    else
    {
      (*data) = (PHIDP_PREPARSED_DATA)kankaku_utils_malloc_or_die((*cbSize), __FILE__, __LINE__);

      winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, (*data), cbSize);
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        fprintf(stderr, "%sGetRawInputDeviceInfo failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
        utils_print_win32_last_error();
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
      utils_print_win32_last_error();

      exit(-1);
    }
    else
    {
      (*pData) = (LPVOID)kankaku_utils_malloc_or_die((*pcbSize), __FILE__, __LINE__);

      winReturnCode = GetRawInputData(hRawInput, RID_INPUT, (*pData), pcbSize, sizeof(RAWINPUTHEADER));
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        printf(FG_RED);
        printf("GetRawInputData failed at %s:%d\n", __FILE__, __LINE__);
        printf(RESET_COLOR);
        utils_print_win32_last_error();

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
