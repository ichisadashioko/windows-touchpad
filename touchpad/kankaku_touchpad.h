#ifndef __TOUCHPAD_H__
#define __TOUCHPAD_H__
#include <Windows.h>
#include <tchar.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <stdint.h>

// https://docs.microsoft.com/en-us/windows-hardware/design/component-guidelines/supporting-usages-in-multitouch-digitizer-drivers

// Digitizer Page (0x0D)
//
#define HID_USAGE_DIGITIZER_CONFIDENCE            ((USAGE)0x47)
#define HID_USAGE_DIGITIZER_WIDTH                 ((USAGE)0x48)
#define HID_USAGE_DIGITIZER_HEIGHT                ((USAGE)0x49)
#define HID_USAGE_DIGITIZER_CONTACT_ID            ((USAGE)0x51)
#define HID_USAGE_DIGITIZER_CONTACT_COUNT         ((USAGE)0x54)
#define HID_USAGE_DIGITIZER_CONTACT_COUNT_MAXIMUM ((USAGE)0x55)

typedef struct
{
  char* ptr;
  unsigned int length;
  size_t bytesCount;
} kankaku_char_string;

typedef struct
{
  wchar_t* ptr;
  unsigned int length;
  size_t bytesCount;
} kankaku_wchar_string;

typedef struct
{
  int left;
  int top;
  int right;
  int bottom;
} kankaku_hid_rectangle;

typedef struct
{
  USHORT linkCollectionId;
  int hasX;
  int hasY;
  int hasContactID;
  int hasTipSwitch;
  int hasContactCount;
  kankaku_hid_rectangle physicalRectangle;
} kankaku_hid_link_collection_info;

typedef struct
{
  kankaku_hid_link_collection_info* entries;
  unsigned int size;
} kankaku_hid_link_collection_info_list;

typedef struct
{
  kankaku_wchar_string name;
  uint16_t width;
  uint16_t height;
  USHORT contactCountLinkCollectionId;
  kankaku_hid_link_collection_info_list contactLinkCollections;
  PHIDP_PREPARSED_DATA preparsedData;
  size_t preparsedDataBytesCount;
} kankaku_hid_touchpad;

typedef struct
{
  kankaku_hid_touchpad* entries;
  unsigned int size;
} kankaku_hid_touchpad_list;

/*
wrapper of GetRawInputDeviceInfoW with RIDI_DEVICENAME
*/
int kankaku_touchpad_get_raw_input_device_name(_In_ HANDLE hDevice, _Out_ wchar_t** deviceName, _Out_ UINT* nameSize, _Out_ size_t* deviceNameCountBytes);

/*
wrapper of GetRawInputDeviceInfoA with RIDI_PREPARSEDDATA
*/
int kankaku_touchpad_get_raw_input_device_preparsed_data(_In_ HANDLE hDevice, _Out_ PHIDP_PREPARSED_DATA* data, _Out_ UINT* cbSize);

/*
wrapper of GetRawInputDeviceList
*/
int kankaku_touchpad_get_raw_input_device_list(_Out_ UINT* numDevices, _Out_ RAWINPUTDEVICELIST** deviceList);

/*
wrapper of GetRawInputData with RID_INPUT
*/
int kankaku_touchpad_get_raw_input_data(_In_ HRAWINPUT hRawInput, _Out_ PUINT pcbSize, _Out_ LPVOID* pData);

int kankaku_touchpad_parse_available_devices(kankaku_hid_touchpad_list* outTouchpadList);

#endif  // __TOUCHPAD_H__
