// clang-format off
#include <Windows.h>
#include <hidusage.h>
#include <hidpi.h>
// clang-format on
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

// https://docs.microsoft.com/en-us/windows-hardware/design/component-guidelines/supporting-usages-in-multitouch-digitizer-drivers

// clang-format off
//
// Digitizer Page (0x0D)
//
#define HID_USAGE_DIGITIZER_CONFIDENCE              ((USAGE) 0x47)
#define HID_USAGE_DIGITIZER_WIDTH                   ((USAGE) 0x48)
#define HID_USAGE_DIGITIZER_HEIGHT                  ((USAGE) 0x49)
#define HID_USAGE_DIGITIZER_CONTACT_ID              ((USAGE) 0x51)
#define HID_USAGE_DIGITIZER_CONTACT_COUNT           ((USAGE) 0x54)
#define HID_USAGE_DIGITIZER_CONTACT_COUNT_MAXIMUM   ((USAGE) 0x55)
// clang-format on

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

void printTimestamp();
void printLastError();
void print_HidP_errors(NTSTATUS, std::string, int);

struct FIND_INPUT_DEVICE_RETVAL {
  HID_DEVICE_INFO_LIST ModifiedList;
  unsigned int FoundIndex;
};

FIND_INPUT_DEVICE_RETVAL
FindInputDeviceInList(HID_DEVICE_INFO_LIST, TCHAR*, const unsigned int, PHIDP_PREPARSED_DATA, const UINT);

struct FIND_LINK_COLLECTION_RETVAL {
  HID_LINK_COL_INFO_LIST ModifiedList;
  unsigned int FoundIndex;
};

FIND_LINK_COLLECTION_RETVAL
FindLinkCollectionInList(HID_LINK_COL_INFO_LIST, USHORT);
