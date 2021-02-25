#ifndef __UTILS_H__
#define __UTILS_H__
#include <Windows.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <tchar.h>

struct HID_TOUCH_LINK_COL_INFO
{
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

typedef struct HID_TOUCH_LINK_COL_INFO HID_TOUCH_LINK_COL_INFO;

struct HID_LINK_COL_INFO_LIST
{
  HID_TOUCH_LINK_COL_INFO* Entries;
  unsigned int Size;
};

typedef struct HID_LINK_COL_INFO_LIST HID_LINK_COL_INFO_LIST;

// C doesn't have map or dictionary so we are going to use array of struct to replace that
struct HID_DEVICE_INFO
{
  TCHAR* Name;
  unsigned int cbName;
  HID_LINK_COL_INFO_LIST LinkColInfoList;
  PHIDP_PREPARSED_DATA PreparedData;
  UINT cbPreparsedData;
  USHORT ContactCountLinkCollection;
};

typedef struct HID_DEVICE_INFO HID_DEVICE_INFO;

struct HID_DEVICE_INFO_LIST
{
  HID_DEVICE_INFO* Entries;
  unsigned int Size;
};

typedef struct HID_DEVICE_INFO_LIST HID_DEVICE_INFO_LIST;

void utils_print_win32_last_error();

void utils_print_hidp_error(NTSTATUS hidpReturnCode, const char* filePath, int lineNumber);

// TODO refactor the create entry if not found feature to another function
int utils_find_input_device_index_by_name(HID_DEVICE_INFO_LIST* hidInfoList, TCHAR* deviceName, const unsigned int cbDeviceName, PHIDP_PREPARSED_DATA preparsedData, const UINT cbPreparsedData, unsigned int* foundHidIndex);

// TODO refactor the create entry if not found feature to another function
int FindLinkCollectionInList(HID_LINK_COL_INFO_LIST* linkColInfoList, USHORT linkCollection, unsigned int* foundLinkColIdx);

void* kankaku_utils_malloc_or_die(size_t size, char* callerFileLocation, int callerLineNumber);

#endif  // __UTILS_H__
