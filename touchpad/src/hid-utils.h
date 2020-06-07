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
struct HID_USAGE {
    int UsagePage;
    int Usage;
};

const HID_USAGE MULTI_TOUCH_DIGITIZER_X                  = {0x01, 0x30};
const HID_USAGE MULTI_TOUCH_DIGITIZER_Y                  = {0x01, 0x31};
const HID_USAGE MULTI_TOUCH_DIGITIZER_CONTACT_IDENTIFIER = {0x0D, 0x51};
const HID_USAGE MULTI_TOUCH_DIGITIZER_TIP_SWITCH         = {0x0D, 0x42};
const HID_USAGE MULTI_TOUCH_DIGITIZER_IN_RANGE           = {0x0D, 0x32};
const HID_USAGE MULTI_TOUCH_CONTACT_COUNT_MAXIMUM        = {0x0D, 0x55};
const HID_USAGE MULTI_TOUCH_CONFIDENCE                   = {0x0D, 0x47};
const HID_USAGE MULTI_TOUCH_WIDTH                        = {0x0D, 0x48};
const HID_USAGE MULTI_TOUCH_HEIGHT                       = {0x0D, 0x30};
const HID_USAGE MULTI_TOUCH_PRESSURE                     = {0x0D, 0x30};

struct HID_LINK_COLLECTION_INFO {
    USHORT LinkCollection;
    RECT PhysicalRect;

    // As we cannot identify which link collection contains which data, we need all these flags to identify which data the link collection contains.
    // https://docs.microsoft.com/en-us/windows-hardware/design/component-guidelines/supporting-usages-in-multitouch-digitizer-drivers

    int HasX                 = 0;
    int HasY                 = 0;
    int HasContactIdentifier = 0;
    int HasTipSwitch         = 0;
    int HasContactCount      = 0;
    int HasConfidence        = 0;
    int HasWidth             = 0;
    int HasHeight            = 0;
    int HasPressure          = 0;
};

struct HID_LINK_COLLECTION_INFO_LIST {
    HID_LINK_COLLECTION_INFO *Entries = NULL;
    unsigned int Size                 = 0;
};

// C doesn't have map or dictionary so we are going to use array of struct to replace that
struct HID_DEVICE_INFO {
    TCHAR *Name                                          = NULL;
    unsigned int NameSizeInBytes                         = 0;
    HID_LINK_COLLECTION_INFO_LIST LinkCollectionInfoList = {NULL, 0};
    PHIDP_PREPARSED_DATA PreparedData                    = NULL;
    UINT PreparsedDataSizeInBytes                        = 0;
};

struct HID_DEVICE_INFO_LIST {
    HID_DEVICE_INFO *Entries = NULL;
    unsigned int Size        = 0;
};

void printTimestamp();
void printLastError();
void print_HidP_errors(NTSTATUS, std::string, int);

struct findOrCreateTouchpadInfo_RETVAL {
    HID_DEVICE_INFO_LIST ModifiedList = {NULL, 0};
    unsigned int FoundIndex           = (unsigned int)-1;
};

findOrCreateTouchpadInfo_RETVAL findOrCreateTouchpadInfo(HID_DEVICE_INFO_LIST, TCHAR *, const unsigned int, PHIDP_PREPARSED_DATA, const UINT);

struct findOrCreateLinkCollectionInfo_RETVAL {
    HID_LINK_COLLECTION_INFO_LIST ModifiedList = {NULL, 0};
    unsigned int FoundIndex                    = (unsigned int)-1;
};

findOrCreateLinkCollectionInfo_RETVAL findOrCreateLinkCollectionInfo(HID_LINK_COLLECTION_INFO_LIST, USHORT);