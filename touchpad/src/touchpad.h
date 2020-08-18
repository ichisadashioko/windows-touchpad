#ifndef __TOUCHPAD_H__
#define __TOUCHPAD_H__
#include <Windows.h>
#include <tchar.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

// https://docs.microsoft.com/en-us/windows-hardware/design/component-guidelines/supporting-usages-in-multitouch-digitizer-drivers

// Digitizer Page (0x0D)
//
#define HID_USAGE_DIGITIZER_CONFIDENCE            ((USAGE)0x47)
#define HID_USAGE_DIGITIZER_WIDTH                 ((USAGE)0x48)
#define HID_USAGE_DIGITIZER_HEIGHT                ((USAGE)0x49)
#define HID_USAGE_DIGITIZER_CONTACT_ID            ((USAGE)0x51)
#define HID_USAGE_DIGITIZER_CONTACT_COUNT         ((USAGE)0x54)
#define HID_USAGE_DIGITIZER_CONTACT_COUNT_MAXIMUM ((USAGE)0x55)

int mGetRawInputDeviceName(_In_ HANDLE hDevice, _Out_ TCHAR** deviceName, _Out_ UINT* nameSize, _Out_ unsigned int* cbDeviceName);
int mGetRawInputDevicePreparsedData(_In_ HANDLE hDevice, _Out_ PHIDP_PREPARSED_DATA* data, _Out_ UINT* cbSize);
int mGetRawInputDeviceList(_Out_ UINT* numDevices, _Out_ RAWINPUTDEVICELIST** deviceList);
int mGetRawInputData(_In_ HRAWINPUT hRawInput, _Out_ PUINT pcbSize, _Out_ LPVOID* pData);
#endif  // __TOUCHPAD_H__
