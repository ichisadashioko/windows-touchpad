// clang-format off
#include <Windows.h>
#include <hidusage.h>
#include <hidpi.h>
// clang-format on
#include <iostream>
#include "termcolor.h"
#include "hid-utils.h"

void printTimestamp() {
  SYSTEMTIME ts;
  GetSystemTime(&ts);
  printf("%s[%04d-%02d-%02d-%02d-%02d-%02d.%03d]%s ", FG_GREEN, ts.wYear,
         ts.wMonth, ts.wDay, ts.wHour, ts.wMinute, ts.wSecond, ts.wMilliseconds,
         RESET_COLOR);
}

void printLastError() {
  DWORD errorCode      = GetLastError();
  LPWSTR messageBuffer = NULL;
  size_t size =
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPWSTR)&messageBuffer, 0, NULL);

  std::cout << FG_RED << "Error code: " << errorCode << std::endl;
  std::wcout << messageBuffer << RESET_COLOR << std::endl;
}

void print_HidP_errors(NTSTATUS hidpReturnCode, std::string filePath,
                       int lineNumber) {
  if (hidpReturnCode == HIDP_STATUS_INVALID_REPORT_LENGTH) {
    std::cout << FG_RED
              << "The report length is not valid. HidP function failed at "
              << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
  } else if (hidpReturnCode == HIDP_STATUS_INVALID_REPORT_TYPE) {
    std::cout << FG_RED
              << "The specified report type is not valid. HidP function "
                 "failed at "
              << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
  } else if (hidpReturnCode == HIDP_STATUS_INCOMPATIBLE_REPORT_ID) {
    std::cout
      << FG_RED
      << "The collection contains a value on the specified usage page in "
         "a report of the specified type, but there are no such usages "
         "in the specified report. HidP function failed at "
      << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
  } else if (hidpReturnCode == HIDP_STATUS_INVALID_PREPARSED_DATA) {
    std::cout << FG_RED
              << "The preparsed data is not valid. HidP function failed at "
              << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
  } else if (hidpReturnCode == HIDP_STATUS_USAGE_NOT_FOUND) {
    std::cout << FG_RED
              << "The collection does not contain a value on the specified "
                 "usage page in any report of the specified report type. "
                 "HidP function failed at "
              << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
  } else {
    std::cout << FG_RED << "Unknown error code: " << hidpReturnCode
              << ". HidP function failed at " << filePath << ":" << lineNumber
              << RESET_COLOR << std::endl;
  }
}

findOrCreateTouchpadInfo_RETVAL
findOrCreateTouchpadInfo(HID_DEVICE_INFO_LIST hidInfoList, TCHAR *deviceName,
                         const unsigned int cbDeviceName,
                         PHIDP_PREPARSED_DATA preparsedData,
                         const UINT cbPreparsedData) {
  findOrCreateTouchpadInfo_RETVAL retval;

  unsigned int foundHidIndex    = (unsigned int)-1;
  HID_DEVICE_INFO *hidInfoArray = hidInfoList.Entries;
  unsigned int hidInfoArraySize = hidInfoList.Size;
  const int isHidArrayNull      = (hidInfoArray == NULL);
  const int isHidArraySizeEmpty = (hidInfoArraySize == 0);
  if (isHidArrayNull || isHidArraySizeEmpty) {
    // the array/list/dictionary is empty
    // allocate memory for the first entry
    hidInfoArraySize = 1;
    foundHidIndex    = 0;

    // TODO recursive free pointers inside struct
    // but we can't know the size of array pointer here
    // because the size is recorded as 0

    // shallow free memory in case if it has been assigned before
    free(hidInfoArray);
    hidInfoArray = (HID_DEVICE_INFO *)malloc(sizeof(HID_DEVICE_INFO));
    if (hidInfoArray == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__
                << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }

    hidInfoArray[foundHidIndex].cbName          = cbDeviceName;
    hidInfoArray[foundHidIndex].LinkColInfoList = {NULL, 0};
    hidInfoArray[foundHidIndex].PreparedData    = preparsedData;
    hidInfoArray[foundHidIndex].cbPreparsedData = cbPreparsedData;

    hidInfoArray[foundHidIndex].Name = (TCHAR *)malloc(cbDeviceName);
    hidInfoArray[foundHidIndex].ContactCountLinkCollection = (USHORT)-1;
    if (hidInfoArray[foundHidIndex].Name == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__
                << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }

    memcpy(hidInfoArray[foundHidIndex].Name, deviceName, cbDeviceName);

    hidInfoArray[foundHidIndex].PreparedData =
      (PHIDP_PREPARSED_DATA)malloc(cbPreparsedData);
    if (hidInfoArray[foundHidIndex].PreparedData == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__
                << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }

    memcpy(hidInfoArray[foundHidIndex].PreparedData, preparsedData,
           cbPreparsedData);
  } else {
    for (unsigned int touchpadIndex = 0; touchpadIndex < hidInfoArraySize;
         touchpadIndex++) {
      int compareNameResult =
        _tcscmp(deviceName, hidInfoArray[touchpadIndex].Name);
      if (compareNameResult == 0) {
        foundHidIndex = touchpadIndex;
        break;
      }
    }
  }

  if (foundHidIndex == (unsigned int)-1) {
    // the array/list/dictionary is not empty
    // but we cannot find any entry with the same name

    // allocate memory and create a new entry at the end of array
    foundHidIndex    = hidInfoArraySize;
    hidInfoArraySize = foundHidIndex + 1;

    // copy entries to new array
    HID_DEVICE_INFO *tmpHidInfoArray =
      (HID_DEVICE_INFO *)malloc(sizeof(HID_DEVICE_INFO) * hidInfoArraySize);
    if (tmpHidInfoArray == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__
                << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }

    for (unsigned int hidIndex = 0; hidIndex < foundHidIndex; hidIndex++) {
      tmpHidInfoArray[hidIndex].Name = hidInfoArray[hidIndex].Name;
      hidInfoArray[hidIndex].Name    = NULL;

      tmpHidInfoArray[hidIndex].cbName = hidInfoArray[hidIndex].cbName;

      tmpHidInfoArray[hidIndex].LinkColInfoList.Entries =
        hidInfoArray[hidIndex].LinkColInfoList.Entries;
      hidInfoArray[hidIndex].LinkColInfoList.Entries = NULL;

      tmpHidInfoArray[hidIndex].LinkColInfoList.Size =
        hidInfoArray[hidIndex].LinkColInfoList.Size;

      tmpHidInfoArray[hidIndex].PreparedData =
        hidInfoArray[hidIndex].PreparedData;
      hidInfoArray[hidIndex].PreparedData = NULL;

      tmpHidInfoArray[hidIndex].cbPreparsedData =
        hidInfoArray[hidIndex].cbPreparsedData;
      tmpHidInfoArray[hidIndex].ContactCountLinkCollection =
        hidInfoArray[hidIndex].ContactCountLinkCollection;
    }

    free(hidInfoArray);
    hidInfoArray = tmpHidInfoArray;

    hidInfoArray[foundHidIndex].cbName          = cbDeviceName;
    hidInfoArray[foundHidIndex].LinkColInfoList = {NULL, 0};
    hidInfoArray[foundHidIndex].cbPreparsedData = cbPreparsedData;

    hidInfoArray[foundHidIndex].Name = (TCHAR *)malloc(cbDeviceName);
    if (hidInfoArray[foundHidIndex].Name == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__
                << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }

    memcpy(hidInfoArray[foundHidIndex].Name, deviceName, cbDeviceName);

    hidInfoArray[foundHidIndex].PreparedData =
      (PHIDP_PREPARSED_DATA)malloc(cbPreparsedData);
    if (hidInfoArray[foundHidIndex].PreparedData == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__
                << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }

    memcpy(hidInfoArray[foundHidIndex].PreparedData, preparsedData,
           cbPreparsedData);
  }

  retval.ModifiedList.Entries = hidInfoArray;
  retval.ModifiedList.Size    = hidInfoArraySize;
  retval.FoundIndex           = foundHidIndex;

  return retval;
}

findOrCreateLinkCollectionInfo_RETVAL
findOrCreateLinkCollectionInfo(HID_LINK_COL_INFO_LIST linkCollectionInfoList,
                               USHORT linkCollection) {
  findOrCreateLinkCollectionInfo_RETVAL retval;
  unsigned int foundLinkCollectionIndex = (unsigned int)-1;
  int isCollectionNull              = (linkCollectionInfoList.Entries == NULL);
  int isCollectionRecordedSizeEmpty = (linkCollectionInfoList.Size == 0);

  if (isCollectionNull || isCollectionRecordedSizeEmpty) {
    foundLinkCollectionIndex    = 0;
    linkCollectionInfoList.Size = (unsigned int)1;

    free(linkCollectionInfoList.Entries);
    linkCollectionInfoList.Entries =
      (HID_TOUCH_LINK_COL_INFO *)malloc(sizeof(HID_TOUCH_LINK_COL_INFO));

    if (linkCollectionInfoList.Entries == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__
                << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }

    linkCollectionInfoList.Entries[foundLinkCollectionIndex].LinkColID =
      linkCollection;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasX          = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasY          = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasTipSwitch  = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasContactID  = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasConfidence = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasWidth      = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasHeight     = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasPressure   = 0;
  } else {
    // std::cout << BG_BLUE << "NumCollections: " <<
    // linkCollectionInfoList.Size << RESET_COLOR << std::endl;
    for (unsigned int linkCollectionIndex = 0;
         linkCollectionIndex < linkCollectionInfoList.Size;
         linkCollectionIndex++) {
      if (linkCollectionInfoList.Entries[linkCollectionIndex].LinkColID ==
          linkCollection) {
        foundLinkCollectionIndex = linkCollectionIndex;
        break;
      }
    }
  }

  if (foundLinkCollectionIndex == (unsigned int)-1) {
    foundLinkCollectionIndex    = linkCollectionInfoList.Size;
    linkCollectionInfoList.Size = foundLinkCollectionIndex + 1;

    HID_TOUCH_LINK_COL_INFO *tmpCollectionArray =
      (HID_TOUCH_LINK_COL_INFO *)malloc(sizeof(HID_TOUCH_LINK_COL_INFO) *
                                        linkCollectionInfoList.Size);
    if (tmpCollectionArray == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__
                << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }

    for (unsigned int linkCollectionIndex = 0;
         linkCollectionIndex < foundLinkCollectionIndex;
         linkCollectionIndex++) {
      tmpCollectionArray[linkCollectionIndex] =
        linkCollectionInfoList.Entries[linkCollectionIndex];
    }

    free(linkCollectionInfoList.Entries);
    linkCollectionInfoList.Entries = tmpCollectionArray;

    linkCollectionInfoList.Entries[foundLinkCollectionIndex].LinkColID =
      linkCollection;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasX          = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasY          = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasTipSwitch  = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasContactID  = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasConfidence = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasWidth      = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasHeight     = 0;
    linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasPressure   = 0;
  }

  retval.ModifiedList = linkCollectionInfoList;
  retval.FoundIndex   = foundLinkCollectionIndex;

  return retval;
}