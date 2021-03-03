#include "kankaku_utils.h"

#include <Windows.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "termcolor.h"

void utils_print_win32_last_error()
{
  DWORD errorCode      = GetLastError();
  LPWSTR messageBuffer = NULL;
  size_t messageSize   = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

  fprintf(stderr, "%serror_code: %d\n", FG_RED, errorCode);
  fwprintf(stderr, messageBuffer);
  fprintf(stderr, "%s\n", RESET_COLOR);
  // TODO check to see if we don't free the messageBuffer pointer, will it lead to memory leaking?
}

void utils_print_hidp_error(NTSTATUS hidpReturnCode, const char* filePath, int lineNumber)
{
  fprintf(stderr, FG_RED);

  if (hidpReturnCode == HIDP_STATUS_INVALID_REPORT_LENGTH)
  {
    fprintf(stderr, "The report length is not valid.");
  }
  else if (hidpReturnCode == HIDP_STATUS_INVALID_REPORT_TYPE)
  {
    fprintf(stderr, "The specified report type is not valid.");
  }
  else if (hidpReturnCode == HIDP_STATUS_INCOMPATIBLE_REPORT_ID)
  {
    fprintf(stderr, "The collection contains a value on the specified usage page in a report of the specified type, but there are no such usages in the specified report.");
  }
  else if (hidpReturnCode == HIDP_STATUS_INVALID_PREPARSED_DATA)
  {
    fprintf(stderr, "The preparsed data is not valid.");
  }
  else if (hidpReturnCode == HIDP_STATUS_USAGE_NOT_FOUND)
  {
    fprintf(stderr, "The collection does not contain a value on the specified usage page in any report of the specified report type.");
  }
  else
  {
    fprintf(stderr, "Unknown error code: %d.", hidpReturnCode);
  }

  fprintf(stderr, " HidP function failed at %s:%d%s\n", filePath, lineNumber, RESET_COLOR);
}

int utils_find_input_device_index_by_name(HID_DEVICE_INFO_LIST* hidInfoList, TCHAR* deviceName, const unsigned int cbDeviceName, PHIDP_PREPARSED_DATA preparsedData, const UINT cbPreparsedData, unsigned int* foundHidIndex)
{
  (*foundHidIndex)              = (unsigned int)-1;
  HID_DEVICE_INFO* hidInfoArray = hidInfoList->Entries;
  unsigned int hidInfoArraySize = hidInfoList->Size;
  const int isHidArrayNull      = (hidInfoArray == NULL);
  const int isHidArraySizeEmpty = (hidInfoArraySize == 0);
  if (isHidArrayNull || isHidArraySizeEmpty)
  {
    // the array/list/dictionary is empty
    // allocate memory for the first entry
    hidInfoArraySize = 1;
    (*foundHidIndex) = 0;

    // TODO recursive free pointers inside struct
    // but we can't know the size of array pointer here
    // because the size is recorded as 0

    // shallow free memory in case if it has been assigned before
    free(hidInfoArray);
    hidInfoArray = (HID_DEVICE_INFO*)kankaku_utils_malloc_or_die(sizeof(HID_DEVICE_INFO), __FILE__, __LINE__);

    hidInfoArray[(*foundHidIndex)].cbName          = cbDeviceName;
    hidInfoArray[(*foundHidIndex)].LinkColInfoList = (kankaku_link_collection_info_list){.Entries = NULL, .Size = 0};
    hidInfoArray[(*foundHidIndex)].PreparedData    = preparsedData;
    hidInfoArray[(*foundHidIndex)].cbPreparsedData = cbPreparsedData;

    hidInfoArray[(*foundHidIndex)].Name                       = (TCHAR*)kankaku_utils_malloc_or_die(cbDeviceName, __FILE__, __LINE__);
    hidInfoArray[(*foundHidIndex)].ContactCountLinkCollection = (USHORT)-1;

    memcpy(hidInfoArray[(*foundHidIndex)].Name, deviceName, cbDeviceName);

    hidInfoArray[(*foundHidIndex)].PreparedData = (PHIDP_PREPARSED_DATA)kankaku_utils_malloc_or_die(cbPreparsedData, __FILE__, __LINE__);

    memcpy(hidInfoArray[(*foundHidIndex)].PreparedData, preparsedData, cbPreparsedData);
  }
  else
  {
    for (unsigned int touchpadIndex = 0; touchpadIndex < hidInfoArraySize; touchpadIndex++)
    {
      int compareNameResult = _tcscmp(deviceName, hidInfoArray[touchpadIndex].Name);
      if (compareNameResult == 0)
      {
        (*foundHidIndex) = touchpadIndex;
        break;
      }
    }
  }

  if ((*foundHidIndex) == (unsigned int)-1)
  {
    // the array/list/dictionary is not empty
    // but we cannot find any entry with the same name

    // allocate memory and create a new entry at the end of array
    (*foundHidIndex) = hidInfoArraySize;
    hidInfoArraySize = (*foundHidIndex) + 1;

    // copy entries to new array
    HID_DEVICE_INFO* tmpHidInfoArray = (HID_DEVICE_INFO*)kankaku_utils_malloc_or_die(sizeof(HID_DEVICE_INFO) * hidInfoArraySize, __FILE__, __LINE__);

    for (unsigned int hidIndex = 0; hidIndex < (*foundHidIndex); hidIndex++)
    {
      tmpHidInfoArray[hidIndex].Name = hidInfoArray[hidIndex].Name;
      hidInfoArray[hidIndex].Name    = NULL;

      tmpHidInfoArray[hidIndex].cbName = hidInfoArray[hidIndex].cbName;

      tmpHidInfoArray[hidIndex].LinkColInfoList.Entries = hidInfoArray[hidIndex].LinkColInfoList.Entries;
      hidInfoArray[hidIndex].LinkColInfoList.Entries    = NULL;

      tmpHidInfoArray[hidIndex].LinkColInfoList.Size = hidInfoArray[hidIndex].LinkColInfoList.Size;

      tmpHidInfoArray[hidIndex].PreparedData = hidInfoArray[hidIndex].PreparedData;
      hidInfoArray[hidIndex].PreparedData    = NULL;

      tmpHidInfoArray[hidIndex].cbPreparsedData            = hidInfoArray[hidIndex].cbPreparsedData;
      tmpHidInfoArray[hidIndex].ContactCountLinkCollection = hidInfoArray[hidIndex].ContactCountLinkCollection;
    }

    free(hidInfoArray);
    hidInfoArray = tmpHidInfoArray;

    hidInfoArray[(*foundHidIndex)].cbName          = cbDeviceName;
    hidInfoArray[(*foundHidIndex)].LinkColInfoList = (kankaku_link_collection_info_list){.Entries = NULL, .Size = 0};
    hidInfoArray[(*foundHidIndex)].cbPreparsedData = cbPreparsedData;

    hidInfoArray[(*foundHidIndex)].Name = (TCHAR*)kankaku_utils_malloc_or_die(cbDeviceName, __FILE__, __LINE__);

    memcpy(hidInfoArray[(*foundHidIndex)].Name, deviceName, cbDeviceName);

    hidInfoArray[(*foundHidIndex)].PreparedData = (PHIDP_PREPARSED_DATA)kankaku_utils_malloc_or_die(cbPreparsedData, __FILE__, __LINE__);

    memcpy(hidInfoArray[(*foundHidIndex)].PreparedData, preparsedData, cbPreparsedData);
  }

  hidInfoList->Entries = hidInfoArray;
  hidInfoList->Size    = hidInfoArraySize;

  return 0;
}

int FindLinkCollectionInList(kankaku_link_collection_info_list* linkColInfoList, USHORT linkCollection, unsigned int* foundLinkColIdx)
{
  (*foundLinkColIdx)      = (unsigned int)-1;
  int isLinkColArrayNull  = (linkColInfoList->Entries == NULL);
  int isLinkColArrayEmpty = (linkColInfoList->Size == 0);

  if (isLinkColArrayNull || isLinkColArrayEmpty)
  {
    (*foundLinkColIdx)    = 0;
    linkColInfoList->Size = 1;

    free(linkColInfoList->Entries);
    linkColInfoList->Entries = (kankaku_link_collection_info*)kankaku_utils_malloc_or_die(sizeof(kankaku_link_collection_info), __FILE__, __LINE__);

    linkColInfoList->Entries[(*foundLinkColIdx)].LinkColID    = linkCollection;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasX         = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasY         = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasTipSwitch = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasContactID = 0;
  }
  else
  {
    for (unsigned int linkColIdx = 0; linkColIdx < linkColInfoList->Size; linkColIdx++)
    {
      if (linkColInfoList->Entries[linkColIdx].LinkColID == linkCollection)
      {
        (*foundLinkColIdx) = linkColIdx;
        break;
      }
    }
  }

  if ((*foundLinkColIdx) == (unsigned int)-1)
  {
    (*foundLinkColIdx)    = linkColInfoList->Size;
    linkColInfoList->Size = (*foundLinkColIdx) + 1;

    kankaku_link_collection_info* tmpCollectionArray = (kankaku_link_collection_info*)kankaku_utils_malloc_or_die(sizeof(kankaku_link_collection_info) * linkColInfoList->Size, __FILE__, __LINE__);

    for (unsigned int linkColIdx = 0; linkColIdx < (*foundLinkColIdx); linkColIdx++)
    {
      tmpCollectionArray[linkColIdx] = linkColInfoList->Entries[linkColIdx];
    }

    free(linkColInfoList->Entries);
    linkColInfoList->Entries = tmpCollectionArray;

    linkColInfoList->Entries[(*foundLinkColIdx)].LinkColID    = linkCollection;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasX         = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasY         = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasTipSwitch = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasContactID = 0;
  }

  return 0;
}

/*
TODO

- store the requested memory size in a list (probably a fixed-size array)
- when free_wrapper is called, find the entry with the same size
- if the entry is found, call `free()` and set that entry to invalid (-1)
- if the entry is not found, return error
*/
static const size_t MAXIMUM_MALLOC_MEMORY = 1 * 1024 * 1024 * 1024;  // 1 GB
static const size_t SIZE_T_MAX            = ~0;
static size_t malloced_memory             = 0;

void* kankaku_utils_malloc_or_die(size_t size, char* callerFileLocation, int callerLineNumber)
{
  // printf("malloc %d bytes for %s:%d\n", size, callerFileLocation, callerLineNumber);

  // check for overflowing
  if ((SIZE_T_MAX - size) < malloced_memory)
  {
    fprintf(stderr, "%sYou are trying to malloc an unreasonable amount of memory! Check your calculation!%s\n", FG_BRIGHT_RED, RESET_COLOR);
    exit(-1);
  }
  else
  {
    malloced_memory += size;
    if (malloced_memory > MAXIMUM_MALLOC_MEMORY)
    {
      fprintf(stderr, "%sExceeded the maximum limit of malloc memory! Check your calculation!%s\n", FG_BRIGHT_RED, RESET_COLOR);
      exit(-1);
    }
  }

  // printf("malloced %d/%d\n", malloced_memory, MAXIMUM_MALLOC_MEMORY);

  void* retval = malloc(size);

  if (retval == NULL)
  {
    fprintf(stderr, "%smalloc failed to allocate %zu byte(s) at %s:%d%s\n", FG_RED, size, callerFileLocation, callerLineNumber, RESET_COLOR);
    exit(-1);
  }

  return retval;
}

void kankaku_utils_free(void* ptr, size_t size, char* callerFileLocation, int callerLineNumber)
{
  if (size > malloced_memory)
  {
    fprintf(stderr, "%sOperation failed! You were trying to free %zu byte(s) which was allocated by us.%s\n", FG_BRIGHT_RED, size, RESET_COLOR);
    fprintf(stderr, "%s%s:%d%s\n", FG_BRIGHT_RED, callerFileLocation, callerLineNumber, RESET_COLOR);
    exit(-1);
  }
  else
  {
    free(ptr);
    malloced_memory -= size;
  }
}
