#pragma once
#include "kankaku_touchpad.h"

#include <Windows.h>
#include <tchar.h>
#include <sal.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <stdio.h>
#include <stdint.h>

#include "termcolor.h"
#include "kankaku_utils.h"

int kankaku_touchpad_get_raw_input_device_name(_In_ HANDLE hDevice, _Out_ TCHAR** deviceName, _Out_ UINT* nameSize, _Out_ size_t* deviceNameCountBytes)
{
  int retval = 0;
  UINT winReturnCode;

#pragma region input checking
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
  else if (deviceNameCountBytes == NULL)
  {
    retval = -1;
    fprintf(stderr, "%scbDeviceName is NULL! You will not able to know the size of the return array%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else
#pragma endregion input checking
  {
    // the actual code is here

    // get the device name size (number of characters)
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
      // size + 1 for NULL terminated string
      (*deviceNameCountBytes) = (sizeof(TCHAR) * ((*nameSize) + 1));
      (*deviceName)           = (TCHAR*)kankaku_utils_malloc_or_die((*deviceNameCountBytes), __FILE__, __LINE__);

      (*deviceName)[(*nameSize)] = 0;

      winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, (*deviceName), nameSize);
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        fprintf(stderr, "%sGetRawInputDeviceInfo failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
        utils_print_win32_last_error();
        kankaku_utils_free((*deviceName), (*deviceNameCountBytes), __FILE__, __LINE__);
        exit(-1);
      }
      else if (winReturnCode != (*nameSize))
      {
        retval = -1;
        fprintf(stderr, "%sGetRawInputDeviceInfo does not return the expected size %d (actual) vs %d (expected) at  %s:%d%s\n", FG_BRIGHT_RED, winReturnCode, (*nameSize), __FILE__, __LINE__, RESET_COLOR);
        kankaku_utils_free((*deviceName), (*deviceNameCountBytes), __FILE__, __LINE__);
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
      size_t deviceListCountBytes = sizeof(RAWINPUTDEVICELIST) * (*numDevices);

      (*deviceList) = (RAWINPUTDEVICELIST*)kankaku_utils_malloc_or_die(deviceListCountBytes, __FILE__, __LINE__);
      winReturnCode = GetRawInputDeviceList((*deviceList), numDevices, sizeof(RAWINPUTDEVICELIST));
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        fprintf(stderr, "%sGetRawInputDeviceList failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
        utils_print_win32_last_error();
        kankaku_utils_free((*deviceList), deviceListCountBytes, __FILE__, __LINE__);
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

int kankaku_touchpad_get_raw_input_data(_In_ HRAWINPUT hRawInput, _Out_ PUINT pcbSize, _Out_ LPVOID* pData)
{
  int retval = 0;
  UINT winReturnCode;

#pragma region input checking
  if (pcbSize == NULL)
  {
    retval = -1;
    fprintf(stderr, "%spcbSize parameter is NULL!%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else if (pData == NULL)
  {
    retval = -1;
    fprintf(stderr, "%s(LPVOID*) pData parameter is NULL!%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else if ((*pData) != NULL)
  {
    retval = -1;
    fprintf(stderr, "%s(LPVOID) pData value is not NULL! Please free your memory and set the pointer value to NULL.%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else
#pragma endregion input checking
  {
    winReturnCode = GetRawInputData(hRawInput, RID_INPUT, NULL, pcbSize, sizeof(RAWINPUTHEADER));
    if (winReturnCode == (UINT)-1)
    {
      retval = -1;
      fprintf(stderr, "%sGetRawInputData failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
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
        fprintf(stderr, "%sGetRawInputData failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
        utils_print_win32_last_error();
        exit(-1);
      }
      else if (winReturnCode != (*pcbSize))
      {
        retval = -1;
        fprintf(stderr, "%sGetRawInputData failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
        fprintf(stderr, "%sThe return value - the number of byte(s) copied into pData (%d) is not equal the expected value (%d)%s\n", FG_BRIGHT_RED, winReturnCode, (*pcbSize), RESET_COLOR);
        exit(-1);
      }
    }
  }

  return retval;
}

int kankaku_touchpad_parse_available_devices()
{
  int retval = 0;

  UINT numberOfDevices;
  // Even though the structure has the word LIST in it, it is not refering to a list but a single element.
  RAWINPUTDEVICELIST* toBeFreedDeviceList = NULL;

  retval = kankaku_touchpad_get_raw_input_device_list(&numberOfDevices, &toBeFreedDeviceList);
  if (retval)
  {
    fprintf(stderr, "%skankaku_touchpad_get_raw_input_device_list failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
    exit(-1);
  }
  else
  {
    for (UINT deviceIndex = 0; deviceIndex < numberOfDevices; deviceIndex++)
    {
      RAWINPUTDEVICELIST rawInputDevice = toBeFreedDeviceList[deviceIndex];

      if (rawInputDevice.dwType != RIM_TYPEHID)
      {
        // skip basic keyboard and mouse type devices
        continue;
      }

      // Now we need to inspect the device for its "capabilities". If the device is a Windows Precision Touch device, it should have these "UsagePage"s and "Usage".
      // HID_USAGE_PAGE_GENERIC
      // - HID_USAGE_GENERIC_X, HID_USAGE_GENERIC_Y
      // HID_USAGE_PAGE_DIGITIZER
      // - HID_USAGE_DIGITIZER_CONTACT_ID
      // - HID_USAGE_DIGITIZER_CONTACT_COUNT
      // - HID_USAGE_DIGITIZER_TIP_SWITCH

      UINT preparsedDataByteCount                 = 0;
      PHIDP_PREPARSED_DATA toBeFreedPreparsedData = NULL;

      retval = kankaku_touchpad_get_raw_input_device_preparsed_data(rawInputDevice.hDevice, &toBeFreedPreparsedData, &preparsedDataByteCount);
      if (retval)
      {
        fprintf(stderr, "%skankaku_touchpad_get_raw_input_device_preparsed_data failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
      }
      else
      {
        NTSTATUS hidpReturnCode;

        HIDP_CAPS caps;
        hidpReturnCode = HidP_GetCaps(toBeFreedPreparsedData, &caps);
        if (hidpReturnCode != HIDP_STATUS_SUCCESS)
        {
          retval = -1;
          utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
        }
        else
        {
          // TODO write up about NumberInputButtonCaps
          int isButtonCapsEmpty = (caps.NumberInputButtonCaps == 0);
          if (!isButtonCapsEmpty)
          {
            UINT deviceNameLength;
            TCHAR* toBeFreedDeviceName = NULL;
            size_t deviceNameCountBytes;

            retval = kankaku_touchpad_get_raw_input_device_name(rawInputDevice.hDevice, &toBeFreedDeviceName, &deviceNameLength, &deviceNameCountBytes);
            if (retval)
            {
              fprintf(stderr, "%skankaku_touchpad_get_raw_input_device_name failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
            }
            else
            {
              // TODO write up about device name's usage
              wprintf(toBeFreedDeviceName);
              printf("\n");
              // WIP
              kankaku_utils_free(toBeFreedDeviceName, deviceNameCountBytes, __FILE__, __LINE__);
            }
          }
        }
        // WIP

        kankaku_utils_free(toBeFreedPreparsedData, preparsedDataByteCount, __FILE__, __LINE__);
      }
    }

    kankaku_utils_free(toBeFreedDeviceList, (sizeof(RAWINPUTDEVICELIST) * numberOfDevices), __FILE__, __LINE__);
  }

  return retval;
}
