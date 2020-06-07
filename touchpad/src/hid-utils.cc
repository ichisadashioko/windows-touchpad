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
    printf("%s[%04d-%02d-%02d-%02d-%02d-%02d.%03d]%s ", FG_GREEN, ts.wYear, ts.wMonth, ts.wDay, ts.wHour, ts.wMinute, ts.wSecond, ts.wMilliseconds, RESET_COLOR);
}

void printLastError() {
    DWORD errorCode     = GetLastError();
    LPSTR messageBuffer = nullptr;
    size_t size         = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    printf("Error code: %d. Error message: %s%s%s", errorCode, FG_RED, messageBuffer, RESET_COLOR);
}

void print_HidP_errors(NTSTATUS hidpReturnCode, std::string filePath, int lineNumber) {
    if (hidpReturnCode == HIDP_STATUS_INVALID_REPORT_LENGTH) {
        std::cout << FG_RED << "The report length is not valid. HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
    } else if (hidpReturnCode == HIDP_STATUS_INVALID_REPORT_TYPE) {
        std::cout << FG_RED << "The specified report type is not valid. HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
    } else if (hidpReturnCode == HIDP_STATUS_INCOMPATIBLE_REPORT_ID) {
        std::cout << FG_RED << "The collection contains a value on the specified usage page in a report of the specified type, but there are no such usages in the specified report. HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR
                  << std::endl;
    } else if (hidpReturnCode == HIDP_STATUS_INVALID_PREPARSED_DATA) {
        std::cout << FG_RED << "The preparsed data is not valid. HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
    } else if (hidpReturnCode == HIDP_STATUS_USAGE_NOT_FOUND) {
        std::cout << FG_RED << "The collection does not contain a value on the specified usage page in any report of the specified report type. HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
    } else {
        std::cout << FG_RED << "Unknown error code: " << hidpReturnCode << ". HidP function failed at " << filePath << ":" << lineNumber << RESET_COLOR << std::endl;
    }
}

findOrCreateTouchpadInfo_RETVAL findOrCreateTouchpadInfo(HID_DEVICE_INFO_LIST touchpadInfoList, TCHAR *deviceName, const unsigned int deviceNameSizeInBytes, PHIDP_PREPARSED_DATA preparsedData, const UINT preparsedDataSizeInBytes) {
    findOrCreateTouchpadInfo_RETVAL retval;

    unsigned int foundTouchpadIndex    = (unsigned int)-1;
    HID_DEVICE_INFO *touchpadInfoArray = touchpadInfoList.Entries;
    unsigned int touchpadInfoArraySize = touchpadInfoList.Size;
    bool isTouchpadsNull               = (touchpadInfoArray == NULL);
    bool isTouchpadsRecordedSizeEmpty  = (touchpadInfoArraySize == 0);
    if (isTouchpadsNull || isTouchpadsRecordedSizeEmpty) {
        // the array/list/dictionary is empty
        // allocate memory for the first entry
        touchpadInfoArraySize = 1;
        foundTouchpadIndex    = 0;

        // TODO recursive free pointers inside struct
        // but we can't know the size of array pointer here
        // because the size is recorded as 0

        // shallow free memory in case if it has been assigned before
        free(touchpadInfoArray);
        touchpadInfoArray = (HID_DEVICE_INFO *)malloc(sizeof(HID_DEVICE_INFO));
        if (touchpadInfoArray == NULL) {
            std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
            exit(-1);
        }

        touchpadInfoArray[foundTouchpadIndex].NameSizeInBytes          = deviceNameSizeInBytes;
        touchpadInfoArray[foundTouchpadIndex].LinkCollectionInfoList   = {NULL, 0};
        touchpadInfoArray[foundTouchpadIndex].PreparedData             = preparsedData;
        touchpadInfoArray[foundTouchpadIndex].PreparsedDataSizeInBytes = preparsedDataSizeInBytes;

        touchpadInfoArray[foundTouchpadIndex].Name = (TCHAR *)malloc(deviceNameSizeInBytes);
        if (touchpadInfoArray[foundTouchpadIndex].Name == NULL) {
            std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
            exit(-1);
        }

        memcpy(touchpadInfoArray[foundTouchpadIndex].Name, deviceName, deviceNameSizeInBytes);

        touchpadInfoArray[foundTouchpadIndex].PreparedData = (PHIDP_PREPARSED_DATA)malloc(preparsedDataSizeInBytes);
        if (touchpadInfoArray[foundTouchpadIndex].PreparedData == NULL) {
            std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
            exit(-1);
        }

        memcpy(touchpadInfoArray[foundTouchpadIndex].PreparedData, preparsedData, preparsedDataSizeInBytes);
    } else {
        for (unsigned int touchpadIndex = 0; touchpadIndex < touchpadInfoArraySize; touchpadIndex++) {
            int compareNameResult = _tcscmp(deviceName, touchpadInfoArray[touchpadIndex].Name);
            if (compareNameResult == 0) {
                foundTouchpadIndex = touchpadIndex;
                break;
            }
        }
    }

    if (foundTouchpadIndex == (unsigned int)-1) {
        // the array/list/dictionary is not empty
        // but we cannot find any entry with the same name

        // allocate memory and create a new entry at the end of array
        foundTouchpadIndex    = touchpadInfoArraySize;
        touchpadInfoArraySize = foundTouchpadIndex + 1;

        // copy entries to new array
        HID_DEVICE_INFO *tmpTouchpadInfoArray = (HID_DEVICE_INFO *)malloc(sizeof(HID_DEVICE_INFO) * touchpadInfoArraySize);
        if (tmpTouchpadInfoArray == NULL) {
            std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
            exit(-1);
        }

        for (unsigned int touchpadIndex = 0; touchpadIndex < foundTouchpadIndex; touchpadIndex++) {
            tmpTouchpadInfoArray[touchpadIndex] = touchpadInfoArray[touchpadIndex];
        }

        free(touchpadInfoArray);
        touchpadInfoArray = tmpTouchpadInfoArray;

        touchpadInfoArray[foundTouchpadIndex].NameSizeInBytes          = deviceNameSizeInBytes;
        touchpadInfoArray[foundTouchpadIndex].LinkCollectionInfoList   = {NULL, 0};
        touchpadInfoArray[foundTouchpadIndex].PreparsedDataSizeInBytes = preparsedDataSizeInBytes;

        touchpadInfoArray[foundTouchpadIndex].Name = (TCHAR *)malloc(deviceNameSizeInBytes);
        if (touchpadInfoArray[foundTouchpadIndex].Name == NULL) {
            std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
            exit(-1);
        }

        memcpy(touchpadInfoArray[foundTouchpadIndex].Name, deviceName, deviceNameSizeInBytes);

        touchpadInfoArray[foundTouchpadIndex].PreparedData = (PHIDP_PREPARSED_DATA)malloc(preparsedDataSizeInBytes);
        if (touchpadInfoArray[foundTouchpadIndex].PreparedData == NULL) {
            std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
            exit(-1);
        }

        memcpy(touchpadInfoArray[foundTouchpadIndex].PreparedData, preparsedData, preparsedDataSizeInBytes);
    }

    retval.ModifiedList.Entries = touchpadInfoArray;
    retval.ModifiedList.Size    = touchpadInfoArraySize;
    retval.FoundIndex           = foundTouchpadIndex;

    return retval;
}

findOrCreateLinkCollectionInfo_RETVAL findOrCreateLinkCollectionInfo(HID_LINK_COLLECTION_INFO_LIST linkCollectionInfoList, USHORT linkCollection) {
    findOrCreateLinkCollectionInfo_RETVAL retval;
    unsigned int foundLinkCollectionIndex = (unsigned int)-1;
    bool isCollectionNull                 = (linkCollectionInfoList.Entries == NULL);
    bool isCollectionRecordedSizeEmpty    = (linkCollectionInfoList.Size == 0);

    if (isCollectionNull || isCollectionRecordedSizeEmpty) {
        foundLinkCollectionIndex    = 0;
        linkCollectionInfoList.Size = (unsigned int)1;

        free(linkCollectionInfoList.Entries);
        linkCollectionInfoList.Entries = (HID_LINK_COLLECTION_INFO *)malloc(sizeof(HID_LINK_COLLECTION_INFO));

        if (linkCollectionInfoList.Entries == NULL) {
            std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
            exit(-1);
        }

        linkCollectionInfoList.Entries[foundLinkCollectionIndex].LinkCollection       = linkCollection;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasX                 = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasY                 = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasTipSwitch         = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasContactIdentifier = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasContactCount      = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasConfidence        = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasWidth             = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasHeight            = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasPressure          = 0;
    } else {
        // std::cout << BG_BLUE << "NumCollections: " << linkCollectionInfoList.Size << RESET_COLOR << std::endl;
        for (unsigned int linkCollectionIndex = 0; linkCollectionIndex < linkCollectionInfoList.Size; linkCollectionIndex++) {
            if (linkCollectionInfoList.Entries[linkCollectionIndex].LinkCollection == linkCollection) {
                foundLinkCollectionIndex = linkCollectionIndex;
                break;
            }
        }
    }

    if (foundLinkCollectionIndex == (unsigned int)-1) {
        foundLinkCollectionIndex    = linkCollectionInfoList.Size;
        linkCollectionInfoList.Size = foundLinkCollectionIndex + 1;

        HID_LINK_COLLECTION_INFO *tmpCollectionArray = (HID_LINK_COLLECTION_INFO *)malloc(sizeof(HID_LINK_COLLECTION_INFO) * linkCollectionInfoList.Size);
        if (tmpCollectionArray == NULL) {
            std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
            exit(-1);
        }

        for (unsigned int linkCollectionIndex = 0; linkCollectionIndex < foundLinkCollectionIndex; linkCollectionIndex++) {
            tmpCollectionArray[linkCollectionIndex] = linkCollectionInfoList.Entries[linkCollectionIndex];
        }

        free(linkCollectionInfoList.Entries);
        linkCollectionInfoList.Entries = tmpCollectionArray;

        linkCollectionInfoList.Entries[foundLinkCollectionIndex].LinkCollection       = linkCollection;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasX                 = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasY                 = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasTipSwitch         = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasContactIdentifier = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasContactCount      = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasConfidence        = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasWidth             = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasHeight            = 0;
        linkCollectionInfoList.Entries[foundLinkCollectionIndex].HasPressure          = 0;
    }

    retval.ModifiedList = linkCollectionInfoList;
    retval.FoundIndex   = foundLinkCollectionIndex;

    return retval;
}