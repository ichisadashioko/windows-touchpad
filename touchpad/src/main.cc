#include <Windows.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <d2d1.h>
#pragma comment(lib, "d2d1.lib")

#include <iostream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <vector>

#include "termcolor.h"
#include "utils.h"
#include "touchpad.h"
#include "touchevents.h"
#include "point2d.h"
#include "stroke.h"

#define LOG_EVERY_INPUT_MESSAGES
#undef LOG_EVERY_INPUT_MESSAGES

static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[]       = _T("use touchpad for handwriting");

static HID_DEVICE_INFO_LIST g_deviceInfoList = {NULL, 0};

static clock_t g_LastRawInputMessageProcessedTime   = 0;
static TCHAR g_LastRawInputMessageDeviceName        = NULL;
static TOUCH_DATA_LIST g_LastRawInputMessageTouches = {NULL, 0};

ID2D1Factory* g_Direct2DFactory                   = NULL;
ID2D1HwndRenderTarget* g_Direct2DHwndRenderTarget = NULL;
ID2D1SolidColorBrush* g_Direct2DSolidColorBrush   = NULL;

static StrokeList g_Strokes    = {NULL, 0};
static ULONG g_TrackingTouchID = (ULONG)-1;

void mParseConnectedInputDevices() {
  std::cout << FG_BLUE << "Parsing all HID devices..." << RESET_COLOR << std::endl;

  // find number of connected devices

  UINT numDevices;
  RAWINPUTDEVICELIST* rawInputDeviceList = NULL;

  mGetRawInputDeviceList(&numDevices, &rawInputDeviceList);

  std::cout << "Number of raw input devices: " << numDevices << std::endl;
  for (UINT deviceIndex = 0; deviceIndex < numDevices; deviceIndex++) {
    std::cout << BG_GREEN << "===== Device #" << deviceIndex << " =====" << RESET_COLOR << std::endl;
    RAWINPUTDEVICELIST rawInputDevice = rawInputDeviceList[deviceIndex];
    if (rawInputDevice.dwType != RIM_TYPEHID) {
      // skip keyboards and mouses
      continue;
    }

    // get preparsed data for HidP
    UINT cbDataSize                    = 0;
    PHIDP_PREPARSED_DATA preparsedData = NULL;

    mGetRawInputDevicePreparsedData(rawInputDevice.hDevice, &preparsedData, &cbDataSize);

    NTSTATUS hidpReturnCode;

    // find HID capabilities
    HIDP_CAPS caps;
    hidpReturnCode = HidP_GetCaps(preparsedData, &caps);
    if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
      print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
      throw;
      exit(-1);
    }

    std::cout << "NumberInputValueCaps: " << caps.NumberInputValueCaps << std::endl;
    std::cout << "NumberInputButtonCaps: " << caps.NumberInputButtonCaps << std::endl;

    int isButtonCapsEmpty = (caps.NumberInputButtonCaps == 0);

    if (!isButtonCapsEmpty) {
      UINT deviceNameLength;
      TCHAR* deviceName = NULL;
      unsigned int cbDeviceName;

      mGetRawInputDeviceName(rawInputDevice.hDevice, &deviceName, &deviceNameLength, &cbDeviceName);

      std::cout << "Device name: ";
      std::wcout << deviceName << std::endl;

      std::cout << FG_GREEN << "Finding device in global list..." << RESET_COLOR << std::endl;
      unsigned int foundHidIdx;
      int returnCode = FindInputDeviceInList(&(g_deviceInfoList), deviceName, cbDeviceName, preparsedData, cbDataSize, &foundHidIdx);
      if (returnCode != 0) {
        std::cout << "FindInputDeviceInList failed at " << __FILE__ << ":" << __LINE__ << std::endl;
        throw;
        exit(-1);
      }

      std::cout << FG_GREEN << "foundHidIdx: " << RESET_COLOR << foundHidIdx << std::endl;

      std::cout << FG_BRIGHT_BLUE << "found device name:   " << RESET_COLOR;
      std::wcout << g_deviceInfoList.Entries[foundHidIdx].Name << std::endl;

      std::cout << FG_BRIGHT_RED << "current device name: " << RESET_COLOR;
      std::wcout << deviceName << std::endl;

      if (caps.NumberInputValueCaps != 0) {
        const USHORT numValueCaps = caps.NumberInputValueCaps;
        USHORT _numValueCaps      = numValueCaps;

        PHIDP_VALUE_CAPS valueCaps = (PHIDP_VALUE_CAPS)mMalloc(sizeof(HIDP_VALUE_CAPS) * numValueCaps, __FILE__, __LINE__);

        hidpReturnCode = HidP_GetValueCaps(HidP_Input, valueCaps, &_numValueCaps, preparsedData);
        if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
          print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
        }

        // x check if numValueCaps value has been changed
        std::cout << "NumberInputValueCaps: " << numValueCaps << " (old) vs " << _numValueCaps << " (new)" << std::endl;

        for (USHORT valueCapIndex = 0; valueCapIndex < numValueCaps; valueCapIndex++) {
          HIDP_VALUE_CAPS cap = valueCaps[valueCapIndex];

          if (cap.IsRange || !cap.IsAbsolute) {
            continue;
          }

          unsigned int foundLinkColIdx;
          int returnCode = FindLinkCollectionInList(&(g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList), cap.LinkCollection, &foundLinkColIdx);
          if (returnCode != 0) {
            std::cout << "FindLinkCollectionInList failed at " << __FILE__ << ":" << __LINE__ << std::endl;
            throw;
            exit(-1);
          }

          std::cout << FG_GREEN << "[ValueCaps] foundLinkCollectionIndex: " << foundLinkColIdx << RESET_COLOR << std::endl;

          if (cap.UsagePage == HID_USAGE_PAGE_GENERIC) {
            std::cout << "=====================================================" << std::endl;
            std::cout << "LinkCollection: " << cap.LinkCollection << std::endl;

            if (cap.NotRange.Usage == HID_USAGE_GENERIC_X) {
              g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].HasX               = 1;
              g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].PhysicalRect.left  = cap.PhysicalMin;
              g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].PhysicalRect.right = cap.PhysicalMax;
              std::cout << "  Left: " << cap.PhysicalMin << std::endl;
              std::cout << "  Right: " << cap.PhysicalMax << std::endl;
            } else if (cap.NotRange.Usage == HID_USAGE_GENERIC_Y) {
              g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].HasY                = 1;
              g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].PhysicalRect.top    = cap.PhysicalMin;
              g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].PhysicalRect.bottom = cap.PhysicalMax;
              std::cout << "  Top: " << cap.PhysicalMin << std::endl;
              std::cout << "  Bottom: " << cap.PhysicalMax << std::endl;
            }
          } else if (cap.UsagePage == HID_USAGE_PAGE_DIGITIZER) {
            if (cap.NotRange.Usage == HID_USAGE_DIGITIZER_CONTACT_ID) {
              g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].HasContactID = 1;
            } else if (cap.NotRange.Usage == HID_USAGE_DIGITIZER_CONTACT_COUNT) {
              g_deviceInfoList.Entries[foundHidIdx].ContactCountLinkCollection = cap.LinkCollection;
            }
          }
        }

        free(valueCaps);
      }

      if (caps.NumberInputButtonCaps != 0) {
        const USHORT numButtonCaps = caps.NumberInputButtonCaps;
        USHORT _numButtonCaps      = numButtonCaps;

        PHIDP_BUTTON_CAPS buttonCaps = (PHIDP_BUTTON_CAPS)mMalloc(sizeof(HIDP_BUTTON_CAPS) * numButtonCaps, __FILE__, __LINE__);

        hidpReturnCode = HidP_GetButtonCaps(HidP_Input, buttonCaps, &_numButtonCaps, preparsedData);
        if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
          print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
          throw;
          exit(-1);
        }

        for (USHORT buttonCapIndex = 0; buttonCapIndex < numButtonCaps; buttonCapIndex++) {
          HIDP_BUTTON_CAPS buttonCap = buttonCaps[buttonCapIndex];

          std::cout << FG_BLUE << "[ButtonCaps] Index: " << buttonCapIndex << ", UsagePage: " << buttonCap.UsagePage << " (DIGITIZER - " << HID_USAGE_PAGE_DIGITIZER << "), IsRange: ";
          printf("%d", buttonCap.IsRange);
          std::cout << RESET_COLOR << std::endl;

          if (buttonCap.IsRange) {
            continue;
          }

          std::cout << BG_GREEN << "Usage: " << buttonCap.NotRange.Usage << RESET_COLOR << std::endl;

          if (buttonCap.UsagePage == HID_USAGE_PAGE_DIGITIZER) {
            if (buttonCap.NotRange.Usage == HID_USAGE_DIGITIZER_TIP_SWITCH) {
              unsigned int foundLinkColIdx;
              int returnCode = FindLinkCollectionInList(&(g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList), buttonCap.LinkCollection, &foundLinkColIdx);

              std::cout << FG_GREEN << "[ButtonCaps] foundLinkCollectionIndex: " << foundLinkColIdx << RESET_COLOR << std::endl;

              g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].HasTipSwitch = 1;
            }
          }
        }

        free(buttonCaps);
      }

      free(deviceName);
    }

    free(preparsedData);
  }

  free(rawInputDeviceList);
}

void mRegisterRawInput(HWND hwnd) {
  // register Windows Precision Touchpad top-level HID collection
  RAWINPUTDEVICE rid;
  clock_t ts = clock();

  rid.usUsagePage = HID_USAGE_PAGE_DIGITIZER;
  rid.usUsage     = HID_USAGE_DIGITIZER_TOUCH_PAD;
  rid.dwFlags     = RIDEV_INPUTSINK;
  rid.hwndTarget  = hwnd;

  if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) {
    std::cout << FG_GREEN << "[" << ts << "]"
              << "Successfully register touchpad!" << RESET_COLOR << std::endl;
  } else {
    std::cout << FG_RED << "[" << ts << "]"
              << "Failed to register touchpad at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
    mGetLastError();
    throw;
    exit(-1);
  }
}

void mHandleCreateMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  HRESULT hr;
  mRegisterRawInput(hwnd);

  hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_Direct2DFactory);
  if (FAILED(hr)) {
    std::cout << FG_RED << "D2D1CreateFactory failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
    throw;
    exit(-1);
  }
}

void mHandleInputMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  UINT winReturnCode;  // return code from windows API call
  clock_t ts = clock();

  // following guide: https://docs.microsoft.com/en-us/windows/win32/inputdev/using-raw-input#performing-a-standard-read-of-raw-input

  // Get the size of RAWINPUT by calling GetRawInputData() with pData = NULL

  UINT rawInputSize = 0;

  winReturnCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &rawInputSize, sizeof(RAWINPUTHEADER));
  if (winReturnCode == (UINT)-1) {
    // handle error
    std::cout << FG_RED << "[" << ts << "]"
              << "GetRawInputData failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
    mGetLastError();

    throw;
    exit(-1);
  }

  // rawInputSize might be modified to 0 after calling GetRawInputData() the second time
  const UINT _rawInputSize = rawInputSize;  // backup the raw input size for checking with return value from GetRawInputData()
  RAWINPUT* rawInputData   = (RAWINPUT*)mMalloc(_rawInputSize, __FILE__, __LINE__);

  winReturnCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawInputData, &rawInputSize, sizeof(RAWINPUTHEADER));
  if (winReturnCode == (UINT)-1) {
    // handle error
    std::cout << FG_RED << "[" << ts << "]"
              << "GetRawInputData failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
    mGetLastError();
    throw;
    exit(-1);
  } else if (winReturnCode != _rawInputSize) {
    std::cout << FG_RED << "[" << ts << "]"
              << "GetRawInputData did not copy enough data as reported at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
    throw;
    exit(-1);
  }

  // Parse the RAWINPUT data.
  if (rawInputData->header.dwType == RIM_TYPEHID) {
    // TODO what does `dwCount` represent?
    DWORD count   = rawInputData->data.hid.dwCount;
    BYTE* rawData = rawInputData->data.hid.bRawData;

    if (count != 0) {
      UINT deviceNameLength;
      TCHAR* deviceName = NULL;
      unsigned int cbDeviceName;

      mGetRawInputDeviceName(rawInputData->header.hDevice, &deviceName, &deviceNameLength, &cbDeviceName);

      unsigned int foundHidIdx = (unsigned int)-1;

      int isTouchpadsNull              = (g_deviceInfoList.Entries == NULL);
      int isTouchpadsRecordedSizeEmpty = (g_deviceInfoList.Size == 0);
      if (isTouchpadsNull || isTouchpadsRecordedSizeEmpty) {
        // TODO parse new connected device data
      } else {
        for (unsigned int touchpadIndex = 0; touchpadIndex < g_deviceInfoList.Size; touchpadIndex++) {
          int compareNameResult = _tcscmp(deviceName, g_deviceInfoList.Entries[touchpadIndex].Name);
          if (compareNameResult == 0) {
            foundHidIdx = touchpadIndex;
            break;
          }
        }
      }

      // std::cout << FG_GREEN << "Device index in stored global array: " << foundTouchpadIndex << RESET_COLOR << std::endl;

      if (foundHidIdx == (unsigned int)-1) {
        // TODO parse new connected device data
      } else {
        int isLinkColArrayNull  = (g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Entries == NULL);
        int isLinkColArrayEmpty = (g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Size == 0);
        int isPreparsedDataNull = (g_deviceInfoList.Entries[foundHidIdx].PreparedData == NULL);

        if (isLinkColArrayNull || isLinkColArrayEmpty) {
          std::cout << FG_RED << "Cannot find any LinkCollection(s). Try parse the PREPARED_DATA may help. TODO" << RESET_COLOR << std::endl;
        } else if (isPreparsedDataNull) {
          std::cout << FG_RED << "Cannot find PreparsedData at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
        } else {
#ifdef LOG_EVERY_INPUT_MESSAGES
          std::cout << "[" << ts << "]" << std::endl;
#endif
          NTSTATUS hidpReturnCode;
          ULONG usageValue;

          PHIDP_PREPARSED_DATA preparsedHIDData = g_deviceInfoList.Entries[foundHidIdx].PreparedData;

          if (g_deviceInfoList.Entries[foundHidIdx].ContactCountLinkCollection == (USHORT)-1) {
            std::cout << FG_RED << "Cannot find contact count Link Collection!" << RESET_COLOR << std::endl;
          } else {
            hidpReturnCode = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_DIGITIZER, g_deviceInfoList.Entries[foundHidIdx].ContactCountLinkCollection, HID_USAGE_DIGITIZER_CONTACT_COUNT, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

            if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
              std::cout << FG_RED << "Failed to read number of contacts!" << RESET_COLOR << std::endl;
              print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
              throw;
              exit(-1);
            }

            ULONG numContacts = usageValue;

#ifdef LOG_EVERY_INPUT_MESSAGES
            std::cout << FG_BRIGHT_BLUE << "numContacts: " << numContacts << RESET_COLOR << std::endl;
#endif

            if (numContacts > g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Size) {
              // TODO how should we deal with this edge case
              std::cout << FG_RED << "number of contacts is greater than Link Collection Array size at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
              throw;
              exit(-1);
            } else {
              for (unsigned int linkColIdx = 0; linkColIdx < numContacts; linkColIdx++) {
                HID_TOUCH_LINK_COL_INFO collectionInfo = g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Entries[linkColIdx];

                if (collectionInfo.HasX && collectionInfo.HasY && collectionInfo.HasContactID && collectionInfo.HasTipSwitch) {
                  hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.LinkColID, 0x30, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

                  if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                    std::cout << FG_RED << "Failed to read x position!" << RESET_COLOR << std::endl;
                    print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                    throw;
                    exit(-1);
                  }

                  ULONG xPos = usageValue;

                  hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.LinkColID, 0x31, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
                  if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                    std::cout << FG_RED << "Failed to read y position!" << RESET_COLOR << std::endl;
                    print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                    throw;
                    exit(-1);
                  }

                  ULONG yPos = usageValue;

                  hidpReturnCode = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.LinkColID, HID_USAGE_DIGITIZER_CONTACT_ID, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
                  if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                    std::cout << FG_RED << "Failed to read touch ID!" << RESET_COLOR << std::endl;
                    print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                    throw;
                    exit(-1);
                  }

                  ULONG touchId = usageValue;

                  const ULONG maxNumButtons = HidP_MaxUsageListLength(HidP_Input, HID_USAGE_PAGE_DIGITIZER, preparsedHIDData);
                  // std::cout << FG_BRIGHT_BLUE << "maximum number of buttons: " << maxNumButtons << RESET_COLOR << std::endl;

                  ULONG _maxNumButtons = maxNumButtons;

                  USAGE* buttonUsageArray = (USAGE*)mMalloc(sizeof(USAGE) * maxNumButtons, __FILE__, __LINE__);

                  hidpReturnCode = HidP_GetUsages(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.LinkColID, buttonUsageArray, &_maxNumButtons, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

                  if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                    std::cout << FG_RED << "HidP_GetUsages failed!" << RESET_COLOR << std::endl;
                    print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                    throw;
                    exit(-1);
                  }

                  int isContactOnSurface = 0;

                  for (ULONG usageIdx = 0; usageIdx < maxNumButtons; usageIdx++) {
                    if (buttonUsageArray[usageIdx] == HID_USAGE_DIGITIZER_TIP_SWITCH) {
                      isContactOnSurface = 1;
                      break;
                    }
                  }

                  free(buttonUsageArray);

                  TOUCH_DATA curTouch;
                  curTouch.TouchID   = touchId;
                  curTouch.X         = xPos;
                  curTouch.Y         = yPos;
                  curTouch.OnSurface = isContactOnSurface;

                  unsigned int touchType;
                  int cStyleFunctionReturnCode = mInterpretRawTouchInput(&g_LastRawInputMessageTouches, curTouch, &touchType);
                  if (cStyleFunctionReturnCode != 0) {
                    std::cout << FG_RED << "mInterpretRawTouchInput failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
                    throw;
                    exit(-1);
                  }

                  Point2D touchPos = {curTouch.X, curTouch.Y};

                  if (g_TrackingTouchID == (ULONG)-1) {
                    if (touchType == EVENT_TYPE_TOUCH_DOWN) {
                      g_TrackingTouchID = curTouch.TouchID;
                      // TODO create new stroke
                      // TODO check return value for indication of errors
                      mCreateNewStroke(touchPos, &g_Strokes);
                      InvalidateRect(hwnd, NULL, NULL);
                    } else {
                      // wait for touch down event to register new stroke
                    }
                  } else if (curTouch.TouchID == g_TrackingTouchID) {
                    if (touchType == EVENT_TYPE_TOUCH_MOVE) {
                      // we skip EVENT_TYPE_TOUCH_MOVE_UNCHANGED here
                      // TODO append touch position to the last stroke
                      if ((g_Strokes.Entries == NULL) || (g_Strokes.Size == 0)) {
                        std::cout << FG_RED << "The application state is broken!" << RESET_COLOR << std::endl;
                        throw;
                        exit(-1);
                      } else {
                        // TODO check return value for indication of errors
                        mAppendPoint2DToList(touchPos, &g_Strokes.Entries[g_Strokes.Size - 1]);
                        InvalidateRect(hwnd, NULL, NULL);
                      }
                    } else if (touchType == EVENT_TYPE_TOUCH_UP) {
                      // I sure that the touch position is the same with the last touch position
                      g_TrackingTouchID = (ULONG)-1;
                    }
                  }

#ifdef LOG_EVERY_INPUT_MESSAGES
                  const char* touchTypeStr;

                  if (touchType == EVENT_TYPE_TOUCH_UP) {
                    touchTypeStr = "touch up";
                  } else if (touchType == EVENT_TYPE_TOUCH_MOVE) {
                    touchTypeStr = "touch move";
                  } else if (touchType == EVENT_TYPE_TOUCH_DOWN) {
                    touchTypeStr = "touch down";
                  } else if (touchType == EVENT_TYPE_TOUCH_MOVE_UNCHANGED) {
                    touchTypeStr = "touch move unchanged";
                  } else {
                    std::cout << FG_RED << "unknown event type: " << touchType << RESET_COLOR << std::endl;
                    throw;
                    exit(-1);
                  }

                  std::cout << FG_GREEN << "LinkColID: " << collectionInfo.LinkColID << " touchID: " << touchId << " | tipSwitch: " << isContactOnSurface << " (" << xPos << ", " << yPos << ") " << touchTypeStr << RESET_COLOR << std::endl;
#endif
                }
              }
            }
          }
        }

        free(deviceName);
      }
    }

    free(rawInputData);
  }
}

HRESULT mCreateGraphicsResources(HWND hwnd) {
  HRESULT hr = S_OK;

  if (g_Direct2DHwndRenderTarget == NULL) {
    RECT rc;
    GetClientRect(hwnd, &rc);

    D2D1_SIZE_U size = {(UINT32)rc.right, (UINT32)rc.bottom};

    if (g_Direct2DFactory == NULL) {
      std::cout << FG_RED << "Direct2DFactory hasn't been initialized!" << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }

    hr = g_Direct2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hwnd, size), &g_Direct2DHwndRenderTarget);

    if (SUCCEEDED(hr)) {
      const D2D1_COLOR_F color = {1.0f, 1.0f, 1.0f, 1.0f};

      hr = g_Direct2DHwndRenderTarget->CreateSolidColorBrush(color, &g_Direct2DSolidColorBrush);
      if (FAILED(hr)) {
        std::cout << FG_RED << "CreateSolidColorBrush failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
        throw;
        exit(-1);
      }
    } else {
      std::cout << FG_RED << "CreateHwndRenderTarget failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }
  }

  return hr;
}

void mDiscardGraphicsResources() {
  // TODO check to see if the order is matter
  if (g_Direct2DHwndRenderTarget != NULL) {
    g_Direct2DHwndRenderTarget->Release();
    g_Direct2DHwndRenderTarget = NULL;
  }

  if (g_Direct2DSolidColorBrush != NULL) {
    g_Direct2DSolidColorBrush->Release();
    g_Direct2DSolidColorBrush = NULL;
  }
}

void mHandlePaintMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
  HRESULT hr = mCreateGraphicsResources(hwnd);
  if (SUCCEEDED(hr)) {
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);

    if (g_Direct2DHwndRenderTarget == NULL) {
      std::cout << FG_RED << "HwndRenderTarget is NULL!" << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    } else {
      g_Direct2DHwndRenderTarget->BeginDraw();
      g_Direct2DHwndRenderTarget->Clear();

      if (g_Direct2DSolidColorBrush == NULL) {
        std::cout << FG_RED << "SolidColorBrush is NULL!" << RESET_COLOR << std::endl;
        throw;
        exit(-1);
      } else {
        if ((g_Strokes.Entries == NULL) || (g_Strokes.Size == 0)) {
        } else {
          for (unsigned int strokeIdx = 0; strokeIdx < g_Strokes.Size; strokeIdx++) {
            // TODO change rendering color for each strokes
            Point2DList strokeData = g_Strokes.Entries[strokeIdx];
            if (strokeData.Size < 2) {
              // TODO improve rendering strokes logic
              continue;
            } else {
              D2D1_POINT_2F firstPoint = {(FLOAT)strokeData.Entries[0].X, (FLOAT)strokeData.Entries[0].Y};
              for (unsigned int pointIdx = 1; pointIdx < strokeData.Size; pointIdx++) {
                D2D1_POINT_2F secondPoint = {(FLOAT)strokeData.Entries[pointIdx].X, (FLOAT)strokeData.Entries[pointIdx].Y};
                g_Direct2DHwndRenderTarget->DrawLine(firstPoint, secondPoint, g_Direct2DSolidColorBrush, 5.0f);
                firstPoint = secondPoint;
              }
            }
          }
        }
      }

      hr = g_Direct2DHwndRenderTarget->EndDraw();
      if (FAILED(hr) || (hr == D2DERR_RECREATE_TARGET)) {
        mDiscardGraphicsResources();
      }
    }

    EndPaint(hwnd, &ps);
  }
}

LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
  switch (uMsg) {
    case WM_CREATE: {
      mHandleCreateMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_INPUT: {
      mHandleInputMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_PAINT: {
      mHandlePaintMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      break;
    }
    default: {
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
  }

  return 0;
}

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
  mParseConnectedInputDevices();

  // TODO detect and prompt user to select touchpad device and parse its width and height
  // default values
  int nWidth  = 720;
  int nHeight = 480;

  if ((g_deviceInfoList.Entries != NULL) && (g_deviceInfoList.Size != 0)) {
    // TODO check for valid touchpad device
    HID_DEVICE_INFO inputDevice = g_deviceInfoList.Entries[0];

    if ((inputDevice.LinkColInfoList.Entries == NULL) || (inputDevice.LinkColInfoList.Size == 0)) {
      std::cout << FG_RED << "Failed to parse link collections of input device!" << RESET_COLOR << std::endl;
      return -1;
    } else {
      for (unsigned int linkColIdx = 0; linkColIdx < inputDevice.LinkColInfoList.Size; linkColIdx++) {
        HID_TOUCH_LINK_COL_INFO linkCollectionInfo = inputDevice.LinkColInfoList.Entries[linkColIdx];
        if (linkCollectionInfo.HasX && linkCollectionInfo.HasY) {
          // TODO Should we need to parse every single touch link collections? For now, I think one is sufficient.
          // TODO validate values (e.g. 0 or > screen size)
          nWidth  = linkCollectionInfo.PhysicalRect.right;
          nHeight = linkCollectionInfo.PhysicalRect.bottom;
          break;
        }
      }
    }
  } else {
    std::cout << FG_RED << "Failed to parse input devices!" << RESET_COLOR << std::endl;
    return -1;
  }

  WNDCLASSEX wcex;

  wcex.cbSize        = sizeof(WNDCLASSEX);
  wcex.style         = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc   = WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = hInstance;
  wcex.hIcon         = LoadIcon(hInstance, IDI_APPLICATION);
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(0);
  wcex.lpszMenuName  = NULL;
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm       = LoadIcon(wcex.hInstance, IDI_APPLICATION);

  if (!RegisterClassEx(&wcex)) {
    std::cout << "RegisterClassEx filed at " << __FILE__ << ":" << __LINE__ << std::endl;
    mGetLastError();
    return -1;
  }

  HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_EX_LAYERED, CW_USEDEFAULT, CW_USEDEFAULT, nWidth, nHeight, NULL, NULL, hInstance, NULL);

  if (!hwnd) {
    std::cout << "CreateWindow filed at " << __FILE__ << ":" << __LINE__ << std::endl;
    mGetLastError();
    return -1;
  }

  // make the window transparent
  // SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
  // SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

int main() {
//#define TEST
#ifdef TEST
  test_mAppendPoint2DToList();
  return 0;
#endif

  return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
};
