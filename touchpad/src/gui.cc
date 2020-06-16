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
#include <vector>

#include "termcolor.h"
#include "hid-utils.h"

// Global variables:
HINSTANCE hInst;  // current instance
int isTouchpadRegistered = 0;
// the main window class name
static TCHAR szWindowClass[] = _T("DesktopApp");
// the application's title bar
static TCHAR szTitle[] = _T("Windows Desktop Guided Tour Application");

static HID_DEVICE_INFO_LIST g_deviceInfoList = {NULL, 0};

// forward declarations of functions included in this code module:
int main();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int);
void handle_wm_create(HWND, UINT, WPARAM, LPARAM);
void handle_wm_input(HWND, UINT, WPARAM, LPARAM);
void ParseConnectedInputDevices();

int CALLBACK wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
  ParseConnectedInputDevices();

  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

  if (!RegisterClassEx(&wcex)) {
    MessageBox(NULL, _T("Call to RegisterClassEx failed!"),
               _T("Windows Desktop Guided Tour"), NULL);
    return 1;
  }

  hInst = hInstance;

  // The parameters to CreateWindow expained:
  // szWindowClass: the name of the application
  // szTitle: the text that appears in the title bar
  // WS_OVERLAPPEDWINDOW: the type of window to create
  // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
  // 500, 100: initial size (width, length)
  // NULL: the parent of this window
  // NULL: this application does not have a menu bar
  // hInstance: the first parameter from WinMain
  // NULL: not used in this application
  HWND hWnd =
      CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                   CW_USEDEFAULT, 500, 100, NULL, NULL, hInstance, NULL);

  if (!hWnd) {
    MessageBox(NULL, _T("Call to CreateWindow failed!"),
               _T("Windows Desktop Guided Tour"), NULL);
    return 1;
  }

  // The parameters to ShowWindow explained:
  // hWnd: the value returned from CreateWindow
  // nCmdShow: the fourth parameter from WinMain
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  // Main message loop:
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

void ParseConnectedInputDevices() {
  UINT winReturnCode;

  std::cout << FG_BLUE << "Retrieving all HID devices..." << RESET_COLOR
            << std::endl;

  // find number of connected devices

  UINT numDevices = 0;
  winReturnCode =
      GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));

  if (winReturnCode == (UINT)-1) {
    std::cout << FG_RED << "GetRawInputDeviceList failed at " << __FILE__ << ":"
              << __LINE__ << RESET_COLOR << std::endl;
    printLastError();
    throw;
    exit(-1);
  }

  std::cout << "Detect " << numDevices << " device(s)!" << std::endl;
  std::unique_ptr<RAWINPUTDEVICELIST[]> rawInputDeviceList(
      new RAWINPUTDEVICELIST[numDevices]);

  // as we have to pass a PUINT to GetRawInputDeviceList the value might
  // be changed.
  const UINT _numDevices = numDevices;
  winReturnCode = GetRawInputDeviceList(rawInputDeviceList.get(), &numDevices,
                                        sizeof(RAWINPUTDEVICELIST));

  if (winReturnCode == (UINT)-1) {
    std::cout << "GetRawInputDeviceList failed at " << __FILE__ << ":"
              << __LINE__ << std::endl;
    printLastError();
    throw;
    exit(-1);
  }

  std::cout << "numDevices: " << numDevices << std::endl;
  for (UINT deviceIndex = 0; deviceIndex < _numDevices; deviceIndex++) {
    std::cout << BG_GREEN << "===== Device #" << deviceIndex
              << " =====" << RESET_COLOR << std::endl;
    RAWINPUTDEVICELIST rawInputDevice = rawInputDeviceList.get()[deviceIndex];
    if (rawInputDevice.dwType != RIM_TYPEHID) {
      // skip keyboards and mouses
      continue;
    }

    // get preparsed data for HidP
    UINT prepasedDataSize = 0;
    winReturnCode = GetRawInputDeviceInfo(
        rawInputDevice.hDevice, RIDI_PREPARSEDDATA, NULL, &prepasedDataSize);
    if (winReturnCode == (UINT)-1) {
      std::cout << "Failed to call GetRawInputDeviceInfo to get prepased data "
                   "size at "
                << __FILE__ ":" << __LINE__ << std::endl;
      printLastError();
      throw;
      exit(-1);
    }

    std::cout << "Prepased data size: " << prepasedDataSize << std::endl;
    const UINT _prepasedDataSize = prepasedDataSize;
    PHIDP_PREPARSED_DATA preparsedData =
        (PHIDP_PREPARSED_DATA)malloc(_prepasedDataSize);
    if (preparsedData == NULL) {
      std::cout << "malloc failed at " << __FILE__ << ":" << __LINE__
                << std::endl;
      throw;
      exit(-1);
    }

    winReturnCode =
        GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_PREPARSEDDATA,
                              preparsedData, &prepasedDataSize);
    if (winReturnCode == (UINT)-1) {
      std::cout
          << "Failed to call GetRawInputDeviceInfo to get prepased data at "
          << __FILE__ << ":" << __LINE__ << std::endl;
      printLastError();
      throw;
      exit(-1);
    }

    NTSTATUS hidpReturnCode;

    // find HID capabilities
    HIDP_CAPS caps;
    hidpReturnCode = HidP_GetCaps(preparsedData, &caps);
    if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
      print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
      throw;
      exit(-1);
    }

    std::cout << "NumberInputValueCaps: " << caps.NumberInputValueCaps
              << std::endl;
    std::cout << "NumberInputButtonCaps: " << caps.NumberInputButtonCaps
              << std::endl;

    int isButtonCapsEmpty = (caps.NumberInputButtonCaps == 0);

    if (!isButtonCapsEmpty) {
      UINT deviceNameBufferSize = 0;
      winReturnCode = GetRawInputDeviceInfo(
          rawInputDevice.hDevice, RIDI_DEVICENAME, NULL, &deviceNameBufferSize);
      if (winReturnCode == (UINT)-1) {
        std::cout << "Failed to call GetRawInputDeviceInfo for "
                     "getting device name size at "
                  << __FILE__ << ":" << __LINE__ << std::endl;
        printLastError();
        throw;
        exit(-1);
      }

      std::cout << "  deviceNameLength: " << deviceNameBufferSize << std::endl;
      // the return value is the number of characters for the device name it is
      // NOT the number of bytes for holding the device name
      const UINT deviceNameLength = deviceNameBufferSize;

      const unsigned int cbDeviceName = sizeof(TCHAR) * (deviceNameLength + 1);
      TCHAR* deviceName = (TCHAR*)malloc(cbDeviceName);

      // set string terminator
      // if we don't do this, the print function will not know when to stop
      deviceName[deviceNameLength] = 0;

      winReturnCode =
          GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_DEVICENAME,
                                deviceName, &deviceNameBufferSize);
      if (winReturnCode == (UINT)-1) {
        std::cout << "Failed to get device name at " << __FILE__ << ":"
                  << __LINE__ << std::endl;
        printLastError();
        throw;
        exit(-1);
      } else if (winReturnCode != deviceNameLength) {
        std::cout << "GetRawInputDeviceInfo does not return the "
                     "expected size "
                  << winReturnCode << " (actual) vs " << deviceNameLength
                  << " (expected) at " << __FILE__ << ":" << __LINE__
                  << std::endl;
        throw;
        exit(-1);
      }

      std::cout << "Device name: ";
      std::wcout << deviceName << std::endl;

      std::cout << FG_GREEN << "Finding device in global list..." << RESET_COLOR
                << std::endl;
      auto findInputDeviceRetval =
          FindInputDeviceInList(g_deviceInfoList, deviceName, cbDeviceName,
                                preparsedData, _prepasedDataSize);
      unsigned int foundHidIdx = findInputDeviceRetval.FoundIndex;
      g_deviceInfoList = findInputDeviceRetval.ModifiedList;

      std::cout << FG_GREEN << "foundHidIdx: " << RESET_COLOR << foundHidIdx
                << std::endl;

      std::cout << FG_BRIGHT_BLUE << "found device name:   " << RESET_COLOR;
      std::wcout << g_deviceInfoList.Entries[foundHidIdx].Name << std::endl;

      std::cout << FG_BRIGHT_RED << "current device name: " << RESET_COLOR;
      std::wcout << deviceName << std::endl;

      if (caps.NumberInputValueCaps != 0) {
        const USHORT numValueCaps = caps.NumberInputValueCaps;
        USHORT _numValueCaps = numValueCaps;

        PHIDP_VALUE_CAPS valueCaps =
            (PHIDP_VALUE_CAPS)malloc(sizeof(HIDP_VALUE_CAPS) * numValueCaps);
        if (valueCaps == NULL) {
          std::cout << "malloc failed at " << __FILE__ << ":" << __LINE__
                    << std::endl;
          throw;
          exit(-1);
        }

        hidpReturnCode = HidP_GetValueCaps(HidP_Input, valueCaps,
                                           &_numValueCaps, preparsedData);
        if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
          print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
        }

        // x check if numValueCaps value has been changed
        std::cout << "NumberInputValueCaps: " << numValueCaps << " (old) vs "
                  << _numValueCaps << " (new)" << std::endl;

        for (USHORT valueCapIndex = 0; valueCapIndex < numValueCaps;
             valueCapIndex++) {
          HIDP_VALUE_CAPS cap = valueCaps[valueCapIndex];

          if (cap.IsRange || !cap.IsAbsolute) {
            continue;
          }

          auto findLinkColRetval = FindLinkCollectionInList(
              g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList,
              cap.LinkCollection);
          g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList =
              findLinkColRetval.ModifiedList;
          const unsigned int foundLinkColIdx = findLinkColRetval.FoundIndex;

          std::cout << FG_GREEN << "[ValueCaps] foundLinkCollectionIndex: "
                    << foundLinkColIdx << RESET_COLOR << std::endl;

          if (cap.UsagePage == HID_USAGE_PAGE_GENERIC) {
            std::cout << "====================================================="
                      << std::endl;
            std::cout << "LinkCollection: " << cap.LinkCollection << std::endl;

            if (cap.NotRange.Usage == HID_USAGE_GENERIC_X) {
              g_deviceInfoList.Entries[foundHidIdx]
                  .LinkColInfoList.Entries[foundLinkColIdx]
                  .HasX = 1;
              g_deviceInfoList.Entries[foundHidIdx]
                  .LinkColInfoList.Entries[foundLinkColIdx]
                  .PhysicalRect.left = cap.PhysicalMin;
              g_deviceInfoList.Entries[foundHidIdx]
                  .LinkColInfoList.Entries[foundLinkColIdx]
                  .PhysicalRect.right = cap.PhysicalMax;
              std::cout << "  Left: " << cap.PhysicalMin << std::endl;
              std::cout << "  Right: " << cap.PhysicalMax << std::endl;
            } else if (cap.NotRange.Usage == HID_USAGE_GENERIC_Y) {
              g_deviceInfoList.Entries[foundHidIdx]
                  .LinkColInfoList.Entries[foundLinkColIdx]
                  .HasY = 1;
              g_deviceInfoList.Entries[foundHidIdx]
                  .LinkColInfoList.Entries[foundLinkColIdx]
                  .PhysicalRect.top = cap.PhysicalMin;
              g_deviceInfoList.Entries[foundHidIdx]
                  .LinkColInfoList.Entries[foundLinkColIdx]
                  .PhysicalRect.bottom = cap.PhysicalMax;
              std::cout << "  Top: " << cap.PhysicalMin << std::endl;
              std::cout << "  Bottom: " << cap.PhysicalMax << std::endl;
            }
          } else if (cap.UsagePage == HID_USAGE_PAGE_DIGITIZER) {
            if (cap.NotRange.Usage == HID_USAGE_DIGITIZER_CONTACT_ID) {
              g_deviceInfoList.Entries[foundHidIdx]
                  .LinkColInfoList.Entries[foundLinkColIdx]
                  .HasContactID = 1;
            } else if (cap.NotRange.Usage ==
                       HID_USAGE_DIGITIZER_CONTACT_COUNT) {
              g_deviceInfoList.Entries[foundHidIdx].ContactCountLinkCollection =
                  cap.LinkCollection;
            }
          }
        }

        free(valueCaps);
      }

      if (caps.NumberInputButtonCaps != 0) {
        const USHORT numButtonCaps = caps.NumberInputButtonCaps;
        USHORT _numButtonCaps = numButtonCaps;

        PHIDP_BUTTON_CAPS buttonCaps =
            (PHIDP_BUTTON_CAPS)malloc(sizeof(HIDP_BUTTON_CAPS) * numButtonCaps);
        if (buttonCaps == NULL) {
          std::cout << "malloc failed at " << __FILE__ << ":" << __LINE__
                    << std::endl;
          throw;
          exit(-1);
        }

        hidpReturnCode = HidP_GetButtonCaps(HidP_Input, buttonCaps,
                                            &_numButtonCaps, preparsedData);
        if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
          print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
          throw;
          exit(-1);
        }

        for (USHORT buttonCapIndex = 0; buttonCapIndex < numButtonCaps;
             buttonCapIndex++) {
          HIDP_BUTTON_CAPS buttonCap = buttonCaps[buttonCapIndex];

          std::cout << FG_BLUE << "[ButtonCaps] Index: " << buttonCapIndex
                    << ", UsagePage: " << buttonCap.UsagePage
                    << " (DIGITIZER - " << HID_USAGE_PAGE_DIGITIZER
                    << "), IsRange: ";
          printf("%d", buttonCap.IsRange);
          std::cout << RESET_COLOR << std::endl;

          if (buttonCap.IsRange) {
            continue;
          }

          std::cout << BG_GREEN << "Usage: " << buttonCap.NotRange.Usage
                    << RESET_COLOR << std::endl;

          if (buttonCap.UsagePage == HID_USAGE_PAGE_DIGITIZER) {
            if (buttonCap.NotRange.Usage == HID_USAGE_DIGITIZER_TIP_SWITCH) {
              auto findLinkColRetval = FindLinkCollectionInList(
                  g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList,
                  buttonCap.LinkCollection);
              g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList =
                  findLinkColRetval.ModifiedList;
              const unsigned int foundLinkColIdx = findLinkColRetval.FoundIndex;

              std::cout << FG_GREEN << "[ButtonCaps] foundLinkCollectionIndex: "
                        << foundLinkColIdx << RESET_COLOR << std::endl;

              g_deviceInfoList.Entries[foundHidIdx]
                  .LinkColInfoList.Entries[foundLinkColIdx]
                  .HasTipSwitch = 1;
            }
          }
        }

        free(buttonCaps);
      }

      free(deviceName);
    }

    free(preparsedData);
  }
}

LRESULT CALLBACK WndProc(_In_ HWND hWnd,
                         _In_ UINT message,
                         _In_ WPARAM wParam,
                         _In_ LPARAM lParam) {
  PAINTSTRUCT ps;
  HDC hdc;
  TCHAR greeting[] = _T("Hello, Windows desktop!");

  switch (message) {
    case WM_CREATE: {
      handle_wm_create(hWnd, message, wParam, lParam);
    } break;
    case WM_INPUT: {
      handle_wm_input(hWnd, message, wParam, lParam);
    } break;
    case WM_PAINT: {
      hdc = BeginPaint(hWnd, &ps);

      // Here your application is laid out.
      // For this introduction, we just print out "Hello, Windows
      // desktop!" in the top left corner.
      TextOut(hdc, 5, 5, greeting, _tcslen(greeting));
      // End application-specific layout section.

      EndPaint(hWnd, &ps);
    } break;
    case WM_DESTROY: {
      PostQuitMessage(0);
    } break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

int main() {
  return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLineW(),
                  SW_SHOWNORMAL);
};

void RegisterDigitigerRawInput(HWND hwnd) {
  // register Windows Precision Touchpad top-level HID collection
  RAWINPUTDEVICE rid;

  rid.usUsagePage = HID_USAGE_PAGE_DIGITIZER;
  rid.usUsage = HID_USAGE_DIGITIZER_TOUCH_PAD;
  rid.dwFlags = RIDEV_INPUTSINK;
  rid.hwndTarget = hwnd;

  if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) {
    isTouchpadRegistered = 1;
    printTimestamp();
    std::cout << FG_GREEN << "Successfully register touchpad!" << RESET_COLOR
              << std::endl;
  } else {
    isTouchpadRegistered = 0;
    printTimestamp();
    std::cout << FG_RED << "Failed to register touchpad at " << __FILE__ << ":"
              << __LINE__ << RESET_COLOR << std::endl;
    printLastError();
    throw;
    exit(-1);
  }
}

void handle_wm_create(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (!isTouchpadRegistered) {
    RegisterDigitigerRawInput(hWnd);
  }
}

void handle_wm_input(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  UINT winReturnCode;  // return code from windows API call

  if (isTouchpadRegistered) {
    // Following guide:
    // https://docs.microsoft.com/en-us/windows/win32/inputdev/using-raw-input#performing-a-standard-read-of-raw-input

    // x should we only read RID_HEADER first to filter device types for
    // RIM_TYPEHID only to save cpu cycles and improve performance

    // Get the size of RAWINPUT by calling GetRawInputData() with pData =
    // NULL

    UINT rawInputSize = 0;

    winReturnCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL,
                                    &rawInputSize, sizeof(RAWINPUTHEADER));
    if (winReturnCode == (UINT)-1) {
      // handle error
      printTimestamp();
      std::cout << FG_RED << "GetRawInputData failed at " << __FILE__ << ":"
                << __LINE__ << RESET_COLOR << std::endl;
      printLastError();

      throw;
      exit(-1);
    }

    // rawInputSize might be modified to 0 after calling GetRawInputData()
    // the second time
    const UINT _rawInputSize =
        rawInputSize;  // backup the raw input size for checking with return
                       // value from GetRawInputData()
    RAWINPUT* rawInputData = (RAWINPUT*)malloc(_rawInputSize);
    if (rawInputData == NULL) {
      std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__
                << std::endl;
      throw;
      exit(-1);
    }

    winReturnCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawInputData,
                                    &rawInputSize, sizeof(RAWINPUTHEADER));
    if (winReturnCode == (UINT)-1) {
      // handle error
      printTimestamp();
      std::cout << FG_RED << "GetRawInputData failed at " << __FILE__ << ":"
                << __LINE__ << RESET_COLOR << std::endl;
      printLastError();
      throw;
      exit(-1);
    } else if (winReturnCode != _rawInputSize) {
      printTimestamp();
      std::cout << FG_RED
                << "GetRawInputData did not copy enough data as reported at "
                << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
      throw;
      exit(-1);
    }

    // Parse the RAWINPUT data.
    if (rawInputData->header.dwType == RIM_TYPEHID) {
      // TODO what does `dwCount` represent?
      DWORD count = rawInputData->data.hid.dwCount;
      BYTE* rawData = rawInputData->data.hid.bRawData;

      if (count != 0) {
        UINT deviceNameBufferSize = 0;
        winReturnCode =
            GetRawInputDeviceInfo(rawInputData->header.hDevice, RIDI_DEVICENAME,
                                  NULL, &deviceNameBufferSize);
        if (winReturnCode == (UINT)-1) {
          std::cout << FG_RED << "GetRawInputDeviceInfo failed at " << __FILE__
                    << ":" << __LINE__ << RESET_COLOR << std::endl;
          printLastError();
          throw;
          exit(-1);
        }

        const UINT deviceNameLength = deviceNameBufferSize;
        TCHAR* deviceName =
            (TCHAR*)malloc(sizeof(TCHAR) * (deviceNameLength + 1));
        if (deviceName == NULL) {
          std::cout << FG_RED << "malloc failed at " << __FILE__ << ":"
                    << __LINE__ << RESET_COLOR << std::endl;
          throw;
          exit(-1);
        }

        deviceName[deviceNameLength] = 0;
        winReturnCode =
            GetRawInputDeviceInfo(rawInputData->header.hDevice, RIDI_DEVICENAME,
                                  deviceName, &deviceNameBufferSize);
        if (winReturnCode == (UINT)-1) {
          std::cout << FG_RED << "GetRawInputDeviceInfo failed at " << __FILE__
                    << ":" << __LINE__ << RESET_COLOR << std::endl;
          printLastError();
          throw;
          exit(-1);
        } else if (winReturnCode != deviceNameLength) {
          std::cout << FG_RED
                    << "GetRawInputDeviceInfo did not copy enough data "
                    << winReturnCode << " (copied) vs " << deviceNameLength
                    << " at " << __FILE__ << ":" << __LINE__ << RESET_COLOR
                    << std::endl;
          throw;
          exit(-1);
        }

        unsigned int foundHidIdx = (unsigned int)-1;

        int isTouchpadsNull = (g_deviceInfoList.Entries == NULL);
        int isTouchpadsRecordedSizeEmpty = (g_deviceInfoList.Size == 0);
        if (isTouchpadsNull || isTouchpadsRecordedSizeEmpty) {
          // TODO parse new connected device data
        } else {
          for (unsigned int touchpadIndex = 0;
               touchpadIndex < g_deviceInfoList.Size; touchpadIndex++) {
            int compareNameResult = _tcscmp(
                deviceName, g_deviceInfoList.Entries[touchpadIndex].Name);
            if (compareNameResult == 0) {
              foundHidIdx = touchpadIndex;
              break;
            }
          }
        }

        // std::cout << FG_GREEN << "Device index in stored global
        // array: " << foundTouchpadIndex << RESET_COLOR << std::endl;

        if (foundHidIdx == (unsigned int)-1) {
          // TODO parse new connected device data
        } else {
          int isLinkColArrayNull =
              (g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Entries ==
               NULL);
          int isLinkColArrayEmpty =
              (g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Size == 0);
          int isPreparsedDataNull =
              (g_deviceInfoList.Entries[foundHidIdx].PreparedData == NULL);

          if (isLinkColArrayNull || isLinkColArrayEmpty) {
            std::cout << FG_RED
                      << "Cannot find any LinkCollection(s). Try parse the "
                         "PREPARED_DATA may help. TODO"
                      << RESET_COLOR << std::endl;
          } else if (isPreparsedDataNull) {
            std::cout << FG_RED << "Cannot find PreparsedData at " << __FILE__
                      << ":" << __LINE__ << RESET_COLOR << std::endl;
          } else {
            std::cout << "=================================" << std::endl;
            NTSTATUS hidpReturnCode;
            ULONG usageValue;

            PHIDP_PREPARSED_DATA preparsedHIDData =
                g_deviceInfoList.Entries[foundHidIdx].PreparedData;

            if (g_deviceInfoList.Entries[foundHidIdx]
                    .ContactCountLinkCollection == (USHORT)-1) {
              std::cout << FG_RED
                        << "Cannot find contact count Link Collection!"
                        << RESET_COLOR << std::endl;
            } else {
              hidpReturnCode = HidP_GetUsageValue(
                  HidP_Input, HID_USAGE_PAGE_DIGITIZER,
                  g_deviceInfoList.Entries[foundHidIdx]
                      .ContactCountLinkCollection,
                  HID_USAGE_DIGITIZER_CONTACT_COUNT, &usageValue,
                  preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData,
                  rawInputData->data.hid.dwSizeHid);

              if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                std::cout << FG_RED << "Failed to read number of contacts!"
                          << RESET_COLOR << std::endl;
                print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                throw;
                exit(-1);
              }

              ULONG numContacts = usageValue;

              std::cout << FG_BRIGHT_BLUE << "numContacts: " << numContacts
                        << RESET_COLOR << std::endl;

              if (numContacts >
                  g_deviceInfoList.Entries[foundHidIdx].LinkColInfoList.Size) {
                std::cout << FG_RED
                          << "number of contacts is greater than Link "
                             "Collection Array size at "
                          << __FILE__ << ":" << __LINE__ << RESET_COLOR
                          << std::endl;
              } else {
                for (unsigned int linkColIdx = 0; linkColIdx < numContacts;
                     linkColIdx++) {
                  HID_TOUCH_LINK_COL_INFO collectionInfo =
                      g_deviceInfoList.Entries[foundHidIdx]
                          .LinkColInfoList.Entries[linkColIdx];

                  if (collectionInfo.HasX && collectionInfo.HasY &&
                      collectionInfo.HasContactID &&
                      collectionInfo.HasTipSwitch) {
                    hidpReturnCode = HidP_GetUsageValue(
                        HidP_Input, 0x01, collectionInfo.LinkColID, 0x30,
                        &usageValue, preparsedHIDData,
                        (PCHAR)rawInputData->data.hid.bRawData,
                        rawInputData->data.hid.dwSizeHid);

                    if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                      std::cout << FG_RED << "Failed to read x position!"
                                << RESET_COLOR << std::endl;
                      print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                      throw;
                      exit(-1);
                    }

                    ULONG xPos = usageValue;

                    hidpReturnCode = HidP_GetUsageValue(
                        HidP_Input, 0x01, collectionInfo.LinkColID, 0x31,
                        &usageValue, preparsedHIDData,
                        (PCHAR)rawInputData->data.hid.bRawData,
                        rawInputData->data.hid.dwSizeHid);
                    if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                      std::cout << FG_RED << "Failed to read y position!"
                                << RESET_COLOR << std::endl;
                      print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                      throw;
                      exit(-1);
                    }

                    ULONG yPos = usageValue;

                    hidpReturnCode = HidP_GetUsageValue(
                        HidP_Input, HID_USAGE_PAGE_DIGITIZER,
                        collectionInfo.LinkColID,
                        HID_USAGE_DIGITIZER_CONTACT_ID, &usageValue,
                        preparsedHIDData,
                        (PCHAR)rawInputData->data.hid.bRawData,
                        rawInputData->data.hid.dwSizeHid);
                    if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                      std::cout << FG_RED << "Failed to read touch ID!"
                                << RESET_COLOR << std::endl;
                      print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                      throw;
                      exit(-1);
                    }

                    ULONG touchId = usageValue;

                    int isContactOnSurface = 0;

                    const ULONG maxNumButtons = HidP_MaxUsageListLength(
                        HidP_Input, HID_USAGE_PAGE_DIGITIZER, preparsedHIDData);
                    std::cout << FG_BRIGHT_BLUE
                              << "maximum number of buttons: " << maxNumButtons
                              << RESET_COLOR << std::endl;

                    ULONG _maxNumButtons = maxNumButtons;

                    USAGE* buttonUsageArray =
                        (USAGE*)malloc(sizeof(USAGE) * maxNumButtons);

                    if (buttonUsageArray == NULL) {
                      std::cout << "malloc failed at " << __FILE__ << ":"
                                << __LINE__ << std::endl;
                      throw;
                      exit(-1);
                    }

                    hidpReturnCode = HidP_GetUsages(
                        HidP_Input, HID_USAGE_PAGE_DIGITIZER,
                        collectionInfo.LinkColID, buttonUsageArray,
                        &_maxNumButtons, preparsedHIDData,
                        (PCHAR)rawInputData->data.hid.bRawData,
                        rawInputData->data.hid.dwSizeHid);

                    if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                      std::cout << FG_RED << "HidP_GetUsages failed!"
                                << RESET_COLOR << std::endl;
                      print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                      throw;
                      exit(-1);
                    }

                    for (ULONG usageIdx = 0; usageIdx < maxNumButtons;
                         usageIdx++) {
                      if (buttonUsageArray[usageIdx] ==
                          HID_USAGE_DIGITIZER_TIP_SWITCH) {
                        isContactOnSurface = 1;
                      }
                    }

                    free(buttonUsageArray);

                    std::cout << FG_GREEN
                              << "LinkColID: " << collectionInfo.LinkColID
                              << " touchID: " << touchId
                              << " | tipSwitch: " << isContactOnSurface << " ("
                              << xPos << ", " << yPos << ")" << RESET_COLOR
                              << std::endl;
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
}