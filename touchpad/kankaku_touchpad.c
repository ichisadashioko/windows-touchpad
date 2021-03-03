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

int kankaku_touchpad_get_raw_input_device_name(_In_ HANDLE hDevice, _Out_ TCHAR** deviceName, _Out_ UINT* nameSize, _Out_ size_t* deviceNameByteCount)
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
  else if (deviceNameByteCount == NULL)
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
      (*deviceNameByteCount) = (sizeof(TCHAR) * ((*nameSize) + 1));
      (*deviceName)          = (TCHAR*)kankaku_utils_malloc_or_die((*deviceNameByteCount), __FILE__, __LINE__);

      (*deviceName)[(*nameSize)] = 0;

      winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, (*deviceName), nameSize);
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        fprintf(stderr, "%sGetRawInputDeviceInfo failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
        utils_print_win32_last_error();
        kankaku_utils_free((*deviceName), (*deviceNameByteCount), __FILE__, __LINE__);
        exit(-1);
      }
      else if (winReturnCode != (*nameSize))
      {
        retval = -1;
        fprintf(stderr, "%sGetRawInputDeviceInfo does not return the expected size %d (actual) vs %d (expected) at  %s:%d%s\n", FG_BRIGHT_RED, winReturnCode, (*nameSize), __FILE__, __LINE__, RESET_COLOR);
        kankaku_utils_free((*deviceName), (*deviceNameByteCount), __FILE__, __LINE__);
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
      size_t deviceListByteCount = sizeof(RAWINPUTDEVICELIST) * (*numDevices);

      (*deviceList) = (RAWINPUTDEVICELIST*)kankaku_utils_malloc_or_die(deviceListByteCount, __FILE__, __LINE__);
      winReturnCode = GetRawInputDeviceList((*deviceList), numDevices, sizeof(RAWINPUTDEVICELIST));
      if (winReturnCode == (UINT)-1)
      {
        retval = -1;
        fprintf(stderr, "%sGetRawInputDeviceList failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
        utils_print_win32_last_error();
        kankaku_utils_free((*deviceList), deviceListByteCount, __FILE__, __LINE__);
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

int kankaku_touchpad_is_link_collection_info_list_not_consistent(kankaku_hid_link_collection_info_list linkCollectionInfoList)
{
  int retval = 0;

  if ((linkCollectionInfoList.entries == NULL) && (linkCollectionInfoList.size != 0))
  {
    fprintf(stderr, "%sentries is NULL but size is not 0%s\n", FG_BRIGHT_RED, RESET_COLOR);
    retval = 1;
  }
  else if ((linkCollectionInfoList.entries != NULL) && (linkCollectionInfoList.size == 0))
  {
    fprintf(stderr, "%sentries is not NULL but size is 0%s\n", FG_BRIGHT_RED, RESET_COLOR);
    retval = 2;
  }

  return retval;
}

int kankaku_touchpad_find_link_collection_info_by_id(USHORT linkCollectionId, kankaku_hid_link_collection_info_list linkCollectionInfoList)
{
  int retval = -1;

  if ((linkCollectionInfoList.entries != NULL) && (linkCollectionInfoList.size > 0))
  {
    for (unsigned int i = 0; i < linkCollectionInfoList.size; i++)
    {
      if (linkCollectionId == linkCollectionInfoList.entries[i].linkCollectionId)
      {
        retval = i;
        break;
      }
    }
  }

  return retval;
}

int kankaku_touchpad_append_new_link_collection_info(USHORT linkCollectionId, kankaku_hid_link_collection_info_list* linkCollectionInfoListPtr)
{
  int retval = -1;

  kankaku_hid_link_collection_info_list linkCollectionInfoList = (*linkCollectionInfoListPtr);

#pragma region input checking
  if (kankaku_touchpad_is_link_collection_info_list_not_consistent(linkCollectionInfoList))
  {
    fprintf(stderr, "%skankaku_hid_link_collection_info_list is not consistent at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
  }
  else
#pragma endregion input checking
  {
    unsigned int newNumberOfEntries                          = linkCollectionInfoList.size + 1;
    size_t previousEntriesByteCount                          = sizeof(kankaku_hid_link_collection_info) * linkCollectionInfoList.size;
    size_t newEntriesByteCount                               = previousEntriesByteCount + sizeof(kankaku_hid_link_collection_info);
    kankaku_hid_link_collection_info* newLinkCollectionArray = kankaku_utils_malloc_or_die(newEntriesByteCount, __FILE__, __LINE__);

    kankaku_hid_link_collection_info newEntry = {.linkCollectionId = linkCollectionId, .hasX = 0, .hasY = 0, .hasContactID = 0, .hasTipSwitch = 0, .physicalRectangle = {.left = -1, .top = -1, .right = -1, .bottom = -1}};

    if (linkCollectionInfoList.size != 0)
    {
      for (unsigned int i = 0; i < linkCollectionInfoList.size; i++)
      {
        newLinkCollectionArray[i] = linkCollectionInfoList.entries[i];
      }

      kankaku_utils_free(linkCollectionInfoList.entries, previousEntriesByteCount, __FILE__, __LINE__);
    }

    linkCollectionInfoList.entries                              = newLinkCollectionArray;
    linkCollectionInfoList.entries[linkCollectionInfoList.size] = newEntry;
    retval                                                      = linkCollectionInfoList.size;
    linkCollectionInfoList.size                                 = newNumberOfEntries;

    (*linkCollectionInfoListPtr) = linkCollectionInfoList;
  }

  return retval;
}

/*
wrap HidP_GetValueCaps and extract the nessesary "value capabilities" from the device
*/
int kankaku_touchpad_parse_value_capability_array(HIDP_CAPS hidCapability, PHIDP_PREPARSED_DATA hidPreparsedData, kankaku_hid_link_collection_info_list* linkCollectionInfoDictPtr)
{
  int retval                                                   = 0;
  kankaku_hid_link_collection_info_list linkCollectionInfoDict = (*linkCollectionInfoDictPtr);

  size_t valueCapabilityArrayByteCount = sizeof(HIDP_VALUE_CAPS) * hidCapability.NumberInputValueCaps;

  PHIDP_VALUE_CAPS toBeFreedValueCapabilityArray = kankaku_utils_malloc_or_die(valueCapabilityArrayByteCount, __FILE__, __LINE__);

  NTSTATUS hidpReturnCode = HidP_GetValueCaps(HidP_Input, toBeFreedValueCapabilityArray, &hidCapability.NumberInputValueCaps, hidPreparsedData);

  if (hidpReturnCode != HIDP_STATUS_SUCCESS)
  {
    retval = -1;
    utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
  }
  else
  {
    for (USHORT valueCapabilityIndex = 0; valueCapabilityIndex < hidCapability.NumberInputValueCaps; valueCapabilityIndex++)
    {
      HIDP_VALUE_CAPS valueCapability = toBeFreedValueCapabilityArray[valueCapabilityIndex];

      // From HID_USAGE_PAGE_GENERIC usage page,
      // - HID_USAGE_GENERIC_X usage can give us the device's width information
      // - HID_USAGE_GENERIC_Y usage can give us the device's height information

      // From HID_USAGE_PAGE_DIGITIZER usage page,
      // - HID_USAGE_DIGITIZER_CONTACT_ID usage // TODO
      // - HID_USAGE_DIGITIZER_CONTACT_COUNT usage can tell us the maximum number of touches that the device can support

      // All of the mentioned usages are not `IsRange` and are `IsAbsolute` // TODO explain why
      if ((!valueCapability.IsRange) && valueCapability.IsAbsolute)
      {
        // In this value capability array, the elements can have the same `LinkCollection`
        int linkCollectionIndex = kankaku_touchpad_find_link_collection_info_by_id(valueCapability.LinkCollection, linkCollectionInfoDict);
        if (linkCollectionIndex < 0)
        {
          linkCollectionIndex = kankaku_touchpad_append_new_link_collection_info(valueCapability.LinkCollection, &linkCollectionInfoDict);
        }

        if (linkCollectionIndex < 0)
        {
          fprintf(stderr, "%skankaku_touchpad_append_new_link_collection_info failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
          retval = -1;
        }
        else
        {
          kankaku_hid_link_collection_info linkCollectionInfo = linkCollectionInfoDict.entries[linkCollectionIndex];

          if (valueCapability.UsagePage == HID_USAGE_PAGE_GENERIC)
          {
            if (valueCapability.NotRange.Usage == HID_USAGE_GENERIC_X)
            {
              linkCollectionInfo.hasX                    = 1;
              linkCollectionInfo.physicalRectangle.left  = valueCapability.PhysicalMin;
              linkCollectionInfo.physicalRectangle.right = valueCapability.PhysicalMax;
            }
            else if (valueCapability.NotRange.Usage == HID_USAGE_GENERIC_Y)
            {
              linkCollectionInfo.hasY                     = 1;
              linkCollectionInfo.physicalRectangle.top    = valueCapability.PhysicalMin;
              linkCollectionInfo.physicalRectangle.bottom = valueCapability.PhysicalMax;
            }
          }
          else if (valueCapability.UsagePage == HID_USAGE_PAGE_DIGITIZER)
          {
            if (valueCapability.NotRange.Usage == HID_USAGE_DIGITIZER_CONTACT_ID)
            {
              linkCollectionInfo.hasContactID = 1;
            }
          }

          linkCollectionInfoDict.entries[linkCollectionIndex] = linkCollectionInfo;
        }
      }

      if (retval)
      {
        break;
      }
    }
  }

  (*linkCollectionInfoDictPtr) = linkCollectionInfoDict;

  kankaku_utils_free(toBeFreedValueCapabilityArray, valueCapabilityArrayByteCount, __FILE__, __LINE__);

  return retval;
}

void print_link_collection_info(kankaku_hid_link_collection_info linkCollectionInfo)
{
  printf("- %slinkCollectionId%s: %d\n", FG_BRIGHT_GREEN, RESET_COLOR, linkCollectionInfo.linkCollectionId);
  printf("- %shasX%s: %d\n", FG_BRIGHT_GREEN, RESET_COLOR, linkCollectionInfo.hasX);
  printf("- %shasY%s: %d\n", FG_BRIGHT_GREEN, RESET_COLOR, linkCollectionInfo.hasY);
  printf("- %shasContactID%s: %d\n", FG_BRIGHT_GREEN, RESET_COLOR, linkCollectionInfo.hasContactID);
  printf("- %shasTipSwitch%s: %d\n", FG_BRIGHT_GREEN, RESET_COLOR, linkCollectionInfo.hasTipSwitch);
  printf("- %sphysicalRectangle%s: (%d, %d, %d, %d)\n", FG_BRIGHT_GREEN, RESET_COLOR, linkCollectionInfo.physicalRectangle.left, linkCollectionInfo.physicalRectangle.top, linkCollectionInfo.physicalRectangle.right, linkCollectionInfo.physicalRectangle.bottom);
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
          // TODO write up about NumberInputButtonCaps and NumberInputValuesCaps
          // We need some values in both of these "caps". Because of that, we can skip devices which do not have these "caps".
          if ((caps.NumberInputButtonCaps != 0) && (caps.NumberInputValueCaps != 0))
          {
            kankaku_hid_link_collection_info_list linkCollectionInfoDict = {.entries = NULL, .size = 0};

            UINT deviceNameLength;
            TCHAR* toBeFreedDeviceName = NULL;
            size_t deviceNameByteCount;

            retval = kankaku_touchpad_get_raw_input_device_name(rawInputDevice.hDevice, &toBeFreedDeviceName, &deviceNameLength, &deviceNameByteCount);
            if (retval)
            {
              fprintf(stderr, "%skankaku_touchpad_get_raw_input_device_name failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
            }
            else
            {
              retval = kankaku_touchpad_parse_value_capability_array(caps, toBeFreedPreparsedData, &linkCollectionInfoDict);

              if (retval)
              {
                fprintf(stderr, "%skankaku_touchpad_parse_value_capability_array failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
              }
              else
              {
                size_t buttonCapabilityArrayByteCount            = caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS);
                PHIDP_BUTTON_CAPS toBeFreedButtonCapabilityArray = kankaku_utils_malloc_or_die(buttonCapabilityArrayByteCount, __FILE__, __LINE__);

                for (USHORT buttonCapabilityIndex = 0; buttonCapabilityIndex < caps.NumberInputButtonCaps; buttonCapabilityIndex++)
                {
                  HIDP_BUTTON_CAPS buttonCapability = toBeFreedButtonCapabilityArray[buttonCapabilityIndex];

                  if (!buttonCapability.IsRange)
                  {
                    if (buttonCapability.UsagePage == HID_USAGE_PAGE_DIGITIZER)
                    {
                      if (buttonCapability.NotRange.Usage == HID_USAGE_DIGITIZER_TIP_SWITCH)
                      {
                        int linkCollectionIndex = kankaku_touchpad_find_link_collection_info_by_id(buttonCapability.LinkCollection, linkCollectionInfoDict);

                        // WIP
                      }
                    }
                  }
                }
                kankaku_utils_free(toBeFreedButtonCapabilityArray, buttonCapabilityArrayByteCount, __FILE__, __LINE__);
              }

              if (retval)
              {
                if (linkCollectionInfoDict.size > 0)
                {
                  kankaku_utils_free(linkCollectionInfoDict.entries, sizeof(kankaku_hid_link_collection_info) * linkCollectionInfoDict.size, __FILE__, __LINE__);
                  linkCollectionInfoDict.entries = NULL;
                  linkCollectionInfoDict.size    = 0;
                }
              }

              // TODO write up about device name's usage
              wprintf(toBeFreedDeviceName);
              printf("\n");
              // WIP
              kankaku_utils_free(toBeFreedDeviceName, deviceNameByteCount, __FILE__, __LINE__);
            }

            for (int i = 0; i < linkCollectionInfoDict.size; i++)
            {
              printf("===============================\n");
              print_link_collection_info(linkCollectionInfoDict.entries[i]);
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
