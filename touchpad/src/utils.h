#pragma once
#ifndef _UTILS_H_
#define _UTILS_H_
#include <Windows.h>
#include <hidusage.h>
#include <hidpi.h>
#include <tchar.h>

// https://docs.microsoft.com/en-us/windows-hardware/design/component-guidelines/supporting-usages-in-multitouch-digitizer-drivers

// Digitizer Page (0x0D)
//
#define HID_USAGE_DIGITIZER_CONFIDENCE            ((USAGE)0x47)
#define HID_USAGE_DIGITIZER_WIDTH                 ((USAGE)0x48)
#define HID_USAGE_DIGITIZER_HEIGHT                ((USAGE)0x49)
#define HID_USAGE_DIGITIZER_CONTACT_ID            ((USAGE)0x51)
#define HID_USAGE_DIGITIZER_CONTACT_COUNT         ((USAGE)0x54)
#define HID_USAGE_DIGITIZER_CONTACT_COUNT_MAXIMUM ((USAGE)0x55)

struct HID_TOUCH_LINK_COL_INFO {
  // LinkColID is an ID to parse HID report
  USHORT LinkColID;
  RECT PhysicalRect;

  // As we cannot identify which link collection contains which data, we need
  // all these flags to identify which data the link collection contains.
  // https://docs.microsoft.com/en-us/windows-hardware/design/component-guidelines/supporting-usages-in-multitouch-digitizer-drivers

  int HasX;
  int HasY;
  int HasContactID;
  int HasTipSwitch;
  int HasConfidence;
  int HasWidth;
  int HasHeight;
  int HasPressure;
};

struct HID_LINK_COL_INFO_LIST {
  HID_TOUCH_LINK_COL_INFO* Entries;
  unsigned int Size;
};

// C doesn't have map or dictionary so we are going to use array of struct to replace that
struct HID_DEVICE_INFO {
  TCHAR* Name;
  unsigned int cbName;
  HID_LINK_COL_INFO_LIST LinkColInfoList;
  PHIDP_PREPARSED_DATA PreparedData;
  UINT cbPreparsedData;
  USHORT ContactCountLinkCollection;
};

struct HID_DEVICE_INFO_LIST {
  HID_DEVICE_INFO* Entries;
  unsigned int Size;
};

void mGetLastError();

void print_HidP_errors(NTSTATUS hidpReturnCode, std::string filePath, int lineNumber);

int FindInputDeviceInList(HID_DEVICE_INFO_LIST* hidInfoList, TCHAR* deviceName, const unsigned int cbDeviceName, PHIDP_PREPARSED_DATA preparsedData, const UINT cbPreparsedData, unsigned int* foundHidIndex);

int FindLinkCollectionInList(HID_LINK_COL_INFO_LIST* linkColInfoList, USHORT linkCollection, unsigned int* foundLinkColIdx);

void* mMalloc(size_t size, std::string filePath, int lineNumber);
#endif  // _UTILS_H_
