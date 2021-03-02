#ifndef __UTILS_H__
#define __UTILS_H__
#include <Windows.h>
#include <tchar.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

/*
TODO What does this structure is for?

This structure is for holding each touches's information. These touches are parsed from the WM_INPUT event.
*/
typedef struct
{
  // LinkColID is an ID to parse HID report; also act as the key
  USHORT LinkColID;

  // TODO I think we can only get the device width and height throught link collection id's info. This is a little bit redundant as all the link collection id's info will probably give the same rectangle.
  RECT PhysicalRect;

  // As we cannot identify which link collection contains which data, we need
  // all these flags to identify which data the link collection contains.
  // https://docs.microsoft.com/en-us/windows-hardware/design/component-guidelines/supporting-usages-in-multitouch-digitizer-drivers

  int HasX;
  int HasY;
  int HasContactID;
  int HasTipSwitch;
} kankaku_link_collection_info;

typedef struct
{
  kankaku_link_collection_info* Entries;
  unsigned int Size;
} kankaku_link_collection_info_list;

// C doesn't have map or dictionary so we are going to use array of struct to replace that
typedef struct
{
  // also act as key
  TCHAR* Name;
  unsigned int cbName;
  kankaku_link_collection_info_list LinkColInfoList;
  PHIDP_PREPARSED_DATA PreparedData;
  UINT cbPreparsedData;
  USHORT ContactCountLinkCollection;
} HID_DEVICE_INFO;

typedef struct
{
  HID_DEVICE_INFO* Entries;
  unsigned int Size;
} HID_DEVICE_INFO_LIST;

void utils_print_win32_last_error();

void utils_print_hidp_error(NTSTATUS hidpReturnCode, const char* filePath, int lineNumber);

// TODO refactor the create entry if not found feature to another function
int utils_find_input_device_index_by_name(HID_DEVICE_INFO_LIST* hidInfoList, TCHAR* deviceName, const unsigned int cbDeviceName, PHIDP_PREPARSED_DATA preparsedData, const UINT cbPreparsedData, unsigned int* foundHidIndex);

// TODO refactor the create entry if not found feature to another function
int FindLinkCollectionInList(kankaku_link_collection_info_list* linkColInfoList, USHORT linkCollection, unsigned int* foundLinkColIdx);

void* kankaku_utils_malloc_or_die(size_t size, char* callerFileLocation, int callerLineNumber);

#endif  // __UTILS_H__
