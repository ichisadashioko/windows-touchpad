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

#include "termcolor.h"
#include "utils.h"

void printLastError() {
  DWORD errorCode      = GetLastError();
  LPWSTR messageBuffer = NULL;
  size_t size          = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

  std::cout << FG_RED << "Error code: " << errorCode << std::endl;
  std::wcout << messageBuffer << RESET_COLOR << std::endl;
}

void print_HidP_errors(NTSTATUS hidpReturnCode, std::string filePath, int lineNumber) {
  if (hidpReturnCode == HIDP_STATUS_INVALID_REPORT_LENGTH) {
    std::cout << FG_RED << "The report length is not valid. HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
  } else if (hidpReturnCode == HIDP_STATUS_INVALID_REPORT_TYPE) {
    std::cout << FG_RED << "The specified report type is not valid. HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
  } else if (hidpReturnCode == HIDP_STATUS_INCOMPATIBLE_REPORT_ID) {
    std::cout << FG_RED << "The collection contains a value on the specified usage page in a report of the specified type, but there are no such usages in the specified report. HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
  } else if (hidpReturnCode == HIDP_STATUS_INVALID_PREPARSED_DATA) {
    std::cout << FG_RED << "The preparsed data is not valid. HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
  } else if (hidpReturnCode == HIDP_STATUS_USAGE_NOT_FOUND) {
    std::cout << FG_RED << "The collection does not contain a value on the specified usage page in any report of the specified report type. HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
  } else {
    std::cout << FG_RED << "Unknown error code: " << hidpReturnCode << ". HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
  }
}

int FindInputDeviceInList(HID_DEVICE_INFO_LIST* hidInfoList, TCHAR* deviceName, const unsigned int cbDeviceName, PHIDP_PREPARSED_DATA preparsedData, const UINT cbPreparsedData, unsigned int* foundHidIndex) {
  (*foundHidIndex)              = (unsigned int)-1;
  HID_DEVICE_INFO* hidInfoArray = hidInfoList->Entries;
  unsigned int hidInfoArraySize = hidInfoList->Size;
  const int isHidArrayNull      = (hidInfoArray == NULL);
  const int isHidArraySizeEmpty = (hidInfoArraySize == 0);
  if (isHidArrayNull || isHidArraySizeEmpty) {
    // the array/list/dictionary is empty
    // allocate memory for the first entry
    hidInfoArraySize = 1;
    (*foundHidIndex) = 0;

    // TODO recursive free pointers inside struct
    // but we can't know the size of array pointer here
    // because the size is recorded as 0

    // shallow free memory in case if it has been assigned before
    free(hidInfoArray);
    hidInfoArray = (HID_DEVICE_INFO*)malloc(sizeof(HID_DEVICE_INFO));
    if (hidInfoArray == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      throw;
      exit(-1);
      return -1;
    }

    hidInfoArray[(*foundHidIndex)].cbName          = cbDeviceName;
    hidInfoArray[(*foundHidIndex)].LinkColInfoList = {NULL, 0};
    hidInfoArray[(*foundHidIndex)].PreparedData    = preparsedData;
    hidInfoArray[(*foundHidIndex)].cbPreparsedData = cbPreparsedData;

    hidInfoArray[(*foundHidIndex)].Name                       = (TCHAR*)malloc(cbDeviceName);
    hidInfoArray[(*foundHidIndex)].ContactCountLinkCollection = (USHORT)-1;
    if (hidInfoArray[(*foundHidIndex)].Name == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      throw;
      exit(-1);
      return -1;
    }

    memcpy(hidInfoArray[(*foundHidIndex)].Name, deviceName, cbDeviceName);

    hidInfoArray[(*foundHidIndex)].PreparedData = (PHIDP_PREPARSED_DATA)malloc(cbPreparsedData);
    if (hidInfoArray[(*foundHidIndex)].PreparedData == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      throw;
      exit(-1);
      return -1;
    }

    memcpy(hidInfoArray[(*foundHidIndex)].PreparedData, preparsedData, cbPreparsedData);
  } else {
    for (unsigned int touchpadIndex = 0; touchpadIndex < hidInfoArraySize; touchpadIndex++) {
      int compareNameResult = _tcscmp(deviceName, hidInfoArray[touchpadIndex].Name);
      if (compareNameResult == 0) {
        (*foundHidIndex) = touchpadIndex;
        break;
      }
    }
  }

  if ((*foundHidIndex) == (unsigned int)-1) {
    // the array/list/dictionary is not empty
    // but we cannot find any entry with the same name

    // allocate memory and create a new entry at the end of array
    (*foundHidIndex) = hidInfoArraySize;
    hidInfoArraySize = (*foundHidIndex) + 1;

    // copy entries to new array
    HID_DEVICE_INFO* tmpHidInfoArray = (HID_DEVICE_INFO*)malloc(sizeof(HID_DEVICE_INFO) * hidInfoArraySize);
    if (tmpHidInfoArray == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      throw;
      exit(-1);
      return -1;
    }

    for (unsigned int hidIndex = 0; hidIndex < (*foundHidIndex); hidIndex++) {
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
    hidInfoArray[(*foundHidIndex)].LinkColInfoList = {NULL, 0};
    hidInfoArray[(*foundHidIndex)].cbPreparsedData = cbPreparsedData;

    hidInfoArray[(*foundHidIndex)].Name = (TCHAR*)malloc(cbDeviceName);
    if (hidInfoArray[(*foundHidIndex)].Name == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      throw;
      exit(-1);
      return -1;
    }

    memcpy(hidInfoArray[(*foundHidIndex)].Name, deviceName, cbDeviceName);

    hidInfoArray[(*foundHidIndex)].PreparedData = (PHIDP_PREPARSED_DATA)malloc(cbPreparsedData);
    if (hidInfoArray[(*foundHidIndex)].PreparedData == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      throw;
      exit(-1);
      return -1;
    }

    memcpy(hidInfoArray[(*foundHidIndex)].PreparedData, preparsedData, cbPreparsedData);
  }

  hidInfoList->Entries = hidInfoArray;
  hidInfoList->Size    = hidInfoArraySize;

  return 0;
}

int FindLinkCollectionInList(HID_LINK_COL_INFO_LIST* linkColInfoList, USHORT linkCollection, unsigned int* foundLinkColIdx) {
  (*foundLinkColIdx)      = (unsigned int)-1;
  int isLinkColArrayNull  = (linkColInfoList->Entries == NULL);
  int isLinkColArrayEmpty = (linkColInfoList->Size == 0);

  if (isLinkColArrayNull || isLinkColArrayEmpty) {
    (*foundLinkColIdx)    = 0;
    linkColInfoList->Size = 1;

    free(linkColInfoList->Entries);
    linkColInfoList->Entries = (HID_TOUCH_LINK_COL_INFO*)malloc(sizeof(HID_TOUCH_LINK_COL_INFO));

    if (linkColInfoList->Entries == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }

    linkColInfoList->Entries[(*foundLinkColIdx)].LinkColID     = linkCollection;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasX          = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasY          = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasTipSwitch  = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasContactID  = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasConfidence = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasWidth      = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasHeight     = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasPressure   = 0;
  } else {
    // std::cout << BG_BLUE << "NumCollections: " << linkColInfoList->Size << RESET_COLOR << std::endl;
    for (unsigned int linkColIdx = 0; linkColIdx < linkColInfoList->Size; linkColIdx++) {
      if (linkColInfoList->Entries[linkColIdx].LinkColID == linkCollection) {
        (*foundLinkColIdx) = linkColIdx;
        break;
      }
    }
  }

  if ((*foundLinkColIdx) == (unsigned int)-1) {
    (*foundLinkColIdx)    = linkColInfoList->Size;
    linkColInfoList->Size = (*foundLinkColIdx) + 1;

    HID_TOUCH_LINK_COL_INFO* tmpCollectionArray = (HID_TOUCH_LINK_COL_INFO*)malloc(sizeof(HID_TOUCH_LINK_COL_INFO) * linkColInfoList->Size);
    if (tmpCollectionArray == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      throw;
      exit(-1);
      return -1;
    }

    for (unsigned int linkColIdx = 0; linkColIdx < (*foundLinkColIdx); linkColIdx++) {
      tmpCollectionArray[linkColIdx] = linkColInfoList->Entries[linkColIdx];
    }

    free(linkColInfoList->Entries);
    linkColInfoList->Entries = tmpCollectionArray;

    linkColInfoList->Entries[(*foundLinkColIdx)].LinkColID     = linkCollection;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasX          = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasY          = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasTipSwitch  = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasContactID  = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasConfidence = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasWidth      = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasHeight     = 0;
    linkColInfoList->Entries[(*foundLinkColIdx)].HasPressure   = 0;
  }

  return 0;
}
