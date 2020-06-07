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
#include <vector>
#include "hid-utils.h"

// Global variables:
HINSTANCE hInst; // current instance
BOOL isTouchpadRegistered          = FALSE;
static TCHAR szWindowClass[]       = _T("DesktopApp");                              // the main window class name
static TCHAR szTitle[]             = _T("Windows Desktop Guided Tour Application"); // the application's title bar
static BOOL showedConnectedDevices = FALSE;

static HID_DEVICE_INFO_LIST g_deviceInfoList = {NULL, 0};

// Forward declarations of functions included in this code module:
int main();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
void WM_CREATE_handle(HWND, UINT, WPARAM, LPARAM);
void WM_INPUT_handle(HWND, UINT, WPARAM, LPARAM);

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    WNDCLASSEX wcex;

    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm       = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Windows Desktop Guided Tour"), NULL);
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
    HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 100, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Windows Desktop Guided Tour"), NULL);
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

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC hdc;
    TCHAR greeting[] = _T("Hello, Windows desktop!");

    switch (message) {
        case WM_CREATE: {
            WM_CREATE_handle(hWnd, message, wParam, lParam);
        } break;
        case WM_INPUT: {
            WM_INPUT_handle(hWnd, message, wParam, lParam);
        } break;
        case WM_PAINT: {
            hdc = BeginPaint(hWnd, &ps);

            // Here your application is laid out.
            // For this introduction, we just print out "Hello, Windows desktop!"
            // in the top left corner.
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

int main() { return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL); };

void WM_CREATE_handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    UINT winReturnCode;

    if (!showedConnectedDevices) {
        showedConnectedDevices = TRUE;

        std::cout << FG_BLUE << "Retrieving all HID devices..." << RESET_COLOR << std::endl;

        // find number of connected devices

        UINT numDevices = 0;
        winReturnCode   = GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));

        if (winReturnCode == (UINT)-1) {
            std::cout << FG_RED << "Failed to get number of HID devices at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
            printLastError();
        } else {
            std::cout << "Detect " << numDevices << " device(s)!" << std::endl;
            std::unique_ptr<RAWINPUTDEVICELIST[]> rawInputDeviceList(new RAWINPUTDEVICELIST[numDevices]);

            // as we have to pass a PUINT to GetRawInputDeviceList the value might be changed.
            const UINT _numDevices = numDevices;
            winReturnCode          = GetRawInputDeviceList(rawInputDeviceList.get(), &numDevices, sizeof(RAWINPUTDEVICELIST));

            if (winReturnCode == (UINT)-1) {
                std::cout << "Failed to call GetRawInputDeviceList at " << __FILE__ << ":" << __LINE__ << std::endl;
                printLastError();
            } else {
                std::cout << "numDevices: " << numDevices << std::endl;
                for (UINT deviceIndex = 0; deviceIndex < _numDevices; deviceIndex++) {
                    std::cout << BG_GREEN << "===== Device #" << deviceIndex << " =====" << RESET_COLOR << std::endl;
                    RAWINPUTDEVICELIST rawInputDevice = rawInputDeviceList.get()[deviceIndex];
                    if (rawInputDevice.dwType != RIM_TYPEHID) {
                        // skip keyboards and mouses
                        continue;
                    }

                    // get preparsed data for HidP
                    UINT prepasedDataSize = 0;
                    winReturnCode         = GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_PREPARSEDDATA, NULL, &prepasedDataSize);
                    if (winReturnCode == (UINT)-1) {
                        std::cout << "Failed to call GetRawInputDeviceInfo to get prepased data size at " << __FILE__ ":" << __LINE__ << std::endl;
                        printLastError();
                        exit(-1);
                    }

                    std::cout << "Prepased data size: " << prepasedDataSize << std::endl;
                    const UINT _prepasedDataSize       = prepasedDataSize;
                    PHIDP_PREPARSED_DATA preparsedData = (PHIDP_PREPARSED_DATA)malloc(_prepasedDataSize);
                    if (preparsedData == NULL) {
                        std::cout << "malloc failed at " << __FILE__ << ":" << __LINE__ << std::endl;
                        exit(-1);
                    }

                    winReturnCode = GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_PREPARSEDDATA, preparsedData, &prepasedDataSize);
                    if (winReturnCode == (UINT)-1) {
                        std::cout << "Failed to call GetRawInputDeviceInfo to get prepased data at " << __FILE__ << ":" << __LINE__ << std::endl;
                        printLastError();
                        exit(-1);
                    }

                    NTSTATUS hidpReturnCode;

                    // find HID capabilities
                    HIDP_CAPS caps;
                    hidpReturnCode = HidP_GetCaps(preparsedData, &caps);
                    if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                        print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                        exit(-1);
                    }

                    std::cout << "NumberInputValueCaps: " << caps.NumberInputValueCaps << std::endl;
                    std::cout << "NumberInputButtonCaps: " << caps.NumberInputButtonCaps << std::endl;

                    // bool isValueCapsEmpty  = (caps.NumberInputValueCaps == 0);
                    bool isButtonCapsEmpty = (caps.NumberInputButtonCaps == 0);

                    if (!isButtonCapsEmpty) {
                        UINT deviceNameBufferSize = 0;
                        winReturnCode             = GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_DEVICENAME, NULL, &deviceNameBufferSize);
                        if (winReturnCode == (UINT)-1) {
                            std::cout << "Failed to call GetRawInputDeviceInfo for getting device name size at " << __FILE__ << ":" << __LINE__ << std::endl;
                            printLastError();
                            exit(-1);
                        }

                        std::cout << "  deviceNameLength: " << deviceNameBufferSize << std::endl;
                        // the return value is the number of characters for the device name
                        // it is NOT the number of bytes for holding the device name
                        const UINT deviceNameLength = deviceNameBufferSize;

                        const unsigned int deviceNameSizeInBytes = sizeof(TCHAR) * (deviceNameLength + 1);
                        TCHAR *deviceName                        = (TCHAR *)malloc(deviceNameSizeInBytes);

                        // set string terminator
                        // if we don't do this, bad thing will happen
                        // the runtime will give all sort of exceptions that does not point to our code
                        deviceName[deviceNameLength] = 0;

                        winReturnCode = GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_DEVICENAME, deviceName, &deviceNameBufferSize);
                        if (winReturnCode == (UINT)-1) {
                            std::cout << "Failed to get device name at " << __FILE__ << ":" << __LINE__ << std::endl;
                            printLastError();
                            exit(-1);
                        } else if (winReturnCode != deviceNameLength) {
                            std::cout << "GetRawInputDeviceInfo does not return the expected size " << winReturnCode << " (actual) vs " << deviceNameLength << " at " << __FILE__ << ":" << __LINE__ << std::endl;
                            exit(-1);
                        }

                        std::cout << "Device name: ";
                        std::wcout << deviceName << std::endl;

                        std::cout << FG_GREEN << "Finding device in global list..." << RESET_COLOR << std::endl;
                        findOrCreateTouchpadInfo_RETVAL findOrCreateTouchpadInfoRetval = findOrCreateTouchpadInfo(g_deviceInfoList, deviceName, deviceNameSizeInBytes, preparsedData, _prepasedDataSize);
                        unsigned int foundTouchpadIndex                                = findOrCreateTouchpadInfoRetval.FoundIndex;
                        g_deviceInfoList                                               = findOrCreateTouchpadInfoRetval.ModifiedList;

                        std::cout << FG_GREEN << "foundTouchpadIndex: " << RESET_COLOR << foundTouchpadIndex << std::endl;

                        std::cout << FG_BRIGHT_BLUE << "found device name:   " << RESET_COLOR;
                        std::wcout << g_deviceInfoList.Entries[foundTouchpadIndex].Name << std::endl;

                        std::cout << FG_BRIGHT_RED << "current device name: " << RESET_COLOR;
                        std::wcout << deviceName << std::endl;

                        if (caps.NumberInputValueCaps != 0) {
                            const USHORT numValueCaps = caps.NumberInputValueCaps;
                            USHORT _numValueCaps      = numValueCaps;

                            PHIDP_VALUE_CAPS valueCaps = (PHIDP_VALUE_CAPS)malloc(sizeof(HIDP_VALUE_CAPS) * numValueCaps);
                            if (valueCaps == NULL) {
                                std::cout << "malloc failed at " << __FILE__ << ":" << __LINE__ << std::endl;
                                exit(-1);
                            }

                            hidpReturnCode = HidP_GetValueCaps(HidP_Input, valueCaps, &_numValueCaps, preparsedData);
                            if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                                print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                            }

                            // x check if numValueCaps value has been changed
                            std::cout << "NumberInputValueCaps: " << numValueCaps << " (old) vs " << _numValueCaps << " (new)" << std::endl;

                            for (USHORT valueCapIndex = 0; valueCapIndex < numValueCaps; valueCapIndex++) {
                                // TODO I don't know what are we doing here

                                HIDP_VALUE_CAPS cap = valueCaps[valueCapIndex];

                                if (cap.IsRange || !cap.IsAbsolute) {
                                    continue;
                                }

                                findOrCreateLinkCollectionInfo_RETVAL findOrCreateLinkCollectionInfoRetval = findOrCreateLinkCollectionInfo(g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList, cap.LinkCollection);
                                g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList        = findOrCreateLinkCollectionInfoRetval.ModifiedList;
                                const unsigned int foundLinkCollectionIndex                                = findOrCreateLinkCollectionInfoRetval.FoundIndex;

                                std::cout << FG_GREEN << "[ValueCaps] foundLinkCollectionIndex: " << foundLinkCollectionIndex << RESET_COLOR << std::endl;

                                if (cap.UsagePage == HID_USAGE_PAGE_GENERIC) {
                                    std::cout << "=================================================" << std::endl;
                                    std::cout << "LinkCollection: " << cap.LinkCollection << std::endl;

                                    if (cap.NotRange.Usage == HID_USAGE_GENERIC_X) {
                                        g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Entries[foundLinkCollectionIndex].HasX               = 1;
                                        g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Entries[foundLinkCollectionIndex].PhysicalRect.left  = cap.PhysicalMin;
                                        g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Entries[foundLinkCollectionIndex].PhysicalRect.right = cap.PhysicalMax;
                                        std::cout << "  Left: " << cap.PhysicalMin << std::endl;
                                        std::cout << "  Right: " << cap.PhysicalMax << std::endl;
                                    } else if (cap.NotRange.Usage == HID_USAGE_GENERIC_Y) {
                                        g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Entries[foundLinkCollectionIndex].HasY                = 1;
                                        g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Entries[foundLinkCollectionIndex].PhysicalRect.top    = cap.PhysicalMin;
                                        g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Entries[foundLinkCollectionIndex].PhysicalRect.bottom = cap.PhysicalMax;
                                        std::cout << "  Top: " << cap.PhysicalMin << std::endl;
                                        std::cout << "  Bottom: " << cap.PhysicalMax << std::endl;
                                    }
                                } else if (cap.UsagePage == HID_USAGE_PAGE_DIGITIZER) {
                                    if (cap.NotRange.Usage == MULTI_TOUCH_DIGITIZER_CONTACT_IDENTIFIER.Usage) {
                                        g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Entries[foundLinkCollectionIndex].HasContactIdentifier = 1;
                                    } else if (cap.NotRange.Usage == MULTI_TOUCH_CONTACT_COUNT_MAXIMUM.Usage) {
                                        g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Entries[foundLinkCollectionIndex].HasContactCount = 1;
                                    }
                                }
                            }

                            free(valueCaps);
                        }

                        if (caps.NumberInputButtonCaps != 0) {
                            const USHORT numButtonCaps = caps.NumberInputButtonCaps;
                            USHORT _numButtonCaps      = numButtonCaps;

                            PHIDP_BUTTON_CAPS buttonCaps = (PHIDP_BUTTON_CAPS)malloc(sizeof(HIDP_BUTTON_CAPS) * numButtonCaps);
                            if (buttonCaps == NULL) {
                                std::cout << "malloc failed at " << __FILE__ << ":" << __LINE__ << std::endl;
                                exit(-1);
                            }

                            hidpReturnCode = HidP_GetButtonCaps(HidP_Input, buttonCaps, &_numButtonCaps, preparsedData);
                            if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                                print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
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
                                    if (buttonCap.NotRange.Usage == MULTI_TOUCH_DIGITIZER_TIP_SWITCH.Usage) {
                                        findOrCreateLinkCollectionInfo_RETVAL findOrCreateLinkCollectionInfoRetval = findOrCreateLinkCollectionInfo(g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList, buttonCap.LinkCollection);
                                        g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList        = findOrCreateLinkCollectionInfoRetval.ModifiedList;
                                        const unsigned int foundLinkCollectionIndex                                = findOrCreateLinkCollectionInfoRetval.FoundIndex;

                                        std::cout << FG_GREEN << "[ButtonCaps] foundLinkCollectionIndex: " << foundLinkCollectionIndex << RESET_COLOR << std::endl;

                                        g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Entries[foundLinkCollectionIndex].HasTipSwitch = 1;
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
        }
    }

    if (!isTouchpadRegistered) {
        // register Windows Precision Touchpad top-level HID collection
        RAWINPUTDEVICE rid;

        rid.usUsagePage = HID_USAGE_PAGE_DIGITIZER;
        rid.usUsage     = HID_USAGE_DIGITIZER_TOUCH_PAD;
        rid.dwFlags     = RIDEV_INPUTSINK;
        rid.hwndTarget  = hWnd;

        if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) {
            isTouchpadRegistered = TRUE;
            printTimestamp();
            std::cout << "Successfully register touchpad!" << std::endl;
        } else {
            isTouchpadRegistered = FALSE;
            printTimestamp();
            std::cout << "Failed to register touchpad at " << __FILE__ << ":" << __LINE__ << std::endl;
            printLastError();
        }
    }
}

void WM_INPUT_handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    UINT winReturnCode; // return code from windows API call

    if (isTouchpadRegistered) {
        // Following guide: https://docs.microsoft.com/en-us/windows/win32/inputdev/using-raw-input#performing-a-standard-read-of-raw-input

        // x should we only read RID_HEADER first to filter device types for RIM_TYPEHID only to save bandwidth and improve performance
        // Get the size of RAWINPUT by calling GetRawInputData() with pData = NULL

        UINT rawInputSize = 0;

        winReturnCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &rawInputSize, sizeof(RAWINPUTHEADER));
        if (winReturnCode == (UINT)-1) {
            // handle error
            printTimestamp();
            std::cout << "Retrieving RAWINPUT size. GetRawInputData() failed at " << __FILE__ << ":" << __LINE__ << std::endl;
            printLastError();
        } else {
            // rawInputSize will be modified to 0 after calling GetRawInputData() the second time
            const UINT _rawInputSize = rawInputSize; // backup the raw input size for checking with return value from GetRawInputData()
            LPBYTE lpb               = new BYTE[rawInputSize];
            if (lpb == NULL) {
                std::cout << "Failed to allocate memory for RAWINPUT at " << __FILE__ << ":" << __LINE__ << std::endl;
                printLastError();
            } else {
                winReturnCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &rawInputSize, sizeof(RAWINPUTHEADER));
                if (winReturnCode == (UINT)-1) {
                    // handle error
                    printTimestamp();
                    std::cout << "Coping RAWINPUT data. GetRawInputData() failed at " << __FILE__ << ":" << __LINE__ << std::endl;
                    printLastError();
                } else if (winReturnCode != _rawInputSize) {
                    printTimestamp();
                    std::cout << "Coping RAWINPUT data. The RAWINPUT size does not match! at " << __FILE__ << ":" << __LINE__ << std::endl;
                } else {
                    // Parse the RAWINPUT data.
                    RAWINPUT *raw = (RAWINPUT *)lpb;
                    if (raw->header.dwType == RIM_TYPEHID) {
                        DWORD sizeHid = raw->data.hid.dwSizeHid;
                        DWORD count   = raw->data.hid.dwCount;
                        BYTE *rawData = raw->data.hid.bRawData;

                        if (count != 0) {
                            // TODO parse raw data

                            // TODO find device name of the input event
                            UINT deviceNameBufferSize = 0;
                            winReturnCode             = GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, NULL, &deviceNameBufferSize);
                            if (winReturnCode == (UINT)-1) {
                                std::cout << FG_RED << "GetRawInputDeviceInfo failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
                                printLastError();
                            } else {
                                const UINT deviceNameLength = deviceNameBufferSize;
                                TCHAR *deviceName           = (TCHAR *)malloc(sizeof(TCHAR) * (deviceNameLength + 1));
                                if (deviceName == NULL) {
                                    std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
                                } else {
                                    deviceName[deviceNameLength] = 0;
                                    winReturnCode                = GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, deviceName, &deviceNameBufferSize);
                                    if (winReturnCode == (UINT)-1) {
                                        std::cout << FG_RED << "GetRawInputDeviceInfo failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
                                        printLastError();
                                    } else if (winReturnCode != deviceNameLength) {
                                        std::cout << FG_RED << "GetRawInputDeviceInfo did not copy enough data " << winReturnCode << " (copied) vs " << deviceNameLength << " at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
                                    } else {
                                        // printTimestamp();
                                        // std::cout << "deviceName: ";
                                        // std::wcout << deviceName << std::endl;

                                        unsigned int foundTouchpadIndex = (unsigned int)-1;

                                        bool isTouchpadsNull              = (g_deviceInfoList.Entries == NULL);
                                        bool isTouchpadsRecordedSizeEmpty = (g_deviceInfoList.Size == 0);
                                        if (isTouchpadsNull || isTouchpadsRecordedSizeEmpty) {
                                            // TODO parse new connected device data
                                        } else {
                                            for (unsigned int touchpadIndex = 0; touchpadIndex < g_deviceInfoList.Size; touchpadIndex++) {
                                                int compareNameResult = _tcscmp(deviceName, g_deviceInfoList.Entries[touchpadIndex].Name);
                                                if (compareNameResult == 0) {
                                                    foundTouchpadIndex = touchpadIndex;
                                                    break;
                                                }
                                            }
                                        }

                                        // std::cout << FG_GREEN << "Device index in stored global array: " << foundTouchpadIndex << RESET_COLOR << std::endl;

                                        if (foundTouchpadIndex == (unsigned int)-1) {
                                            // TODO parse new connected device data
                                        } else {
                                            bool isCollectionNull              = (g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Entries == NULL);
                                            bool isCollectionRecordedSizeEmpty = (g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Size == 0);
                                            bool isPreparsedDataNull           = (g_deviceInfoList.Entries[foundTouchpadIndex].PreparedData == NULL);

                                            if (isCollectionNull || isCollectionRecordedSizeEmpty) {
                                                std::cout << FG_RED << "Cannot find any LinkCollection(s). Try parse the PREPARED_DATA may help. TODO" << RESET_COLOR << std::endl;
                                            } else if (isPreparsedDataNull) {
                                                std::cout << FG_RED << "Cannot find PreparsedData at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
                                            } else {
                                                NTSTATUS hidpReturnCode;
                                                for (unsigned int linkCollectionIndex = 0; linkCollectionIndex < g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Size; linkCollectionIndex++) {
                                                    HID_LINK_COLLECTION_INFO collectionInfo = g_deviceInfoList.Entries[foundTouchpadIndex].LinkCollectionInfoList.Entries[linkCollectionIndex];

                                                    if (collectionInfo.HasX && collectionInfo.HasY && collectionInfo.HasContactIdentifier && collectionInfo.HasTipSwitch) {
                                                        ULONG xPos = 0;
                                                        ULONG yPos = 0;
                                                        ULONG usageValue;

                                                        // std::cout << BG_BLUE << "LinkCollection: " << collectionInfo.LinkCollection << ", HasX: " << collectionInfo.HasX << ", HasY:" << collectionInfo.HasY << RESET_COLOR << std::endl;

                                                        hidpReturnCode = HidP_GetUsageValue(HidP_Input, MULTI_TOUCH_DIGITIZER_X.UsagePage, collectionInfo.LinkCollection, MULTI_TOUCH_DIGITIZER_X.Usage, &usageValue,
                                                                                            g_deviceInfoList.Entries[foundTouchpadIndex].PreparedData, (PCHAR)raw->data.hid.bRawData, raw->data.hid.dwSizeHid);

                                                        if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                                                            std::cout << FG_RED << "Failed to read x position!" << RESET_COLOR << std::endl;
                                                        } else {
                                                            xPos = usageValue;
                                                        }

                                                        hidpReturnCode = HidP_GetUsageValue(HidP_Input, MULTI_TOUCH_DIGITIZER_Y.UsagePage, collectionInfo.LinkCollection, MULTI_TOUCH_DIGITIZER_Y.Usage, &usageValue,
                                                                                            g_deviceInfoList.Entries[foundTouchpadIndex].PreparedData, (PCHAR)raw->data.hid.bRawData, raw->data.hid.dwSizeHid);

                                                        if (hidpReturnCode != HIDP_STATUS_SUCCESS) {
                                                            std::cout << FG_RED << "Failed to read y position!" << RESET_COLOR << std::endl;
                                                        } else {
                                                            yPos = usageValue;
                                                        }

                                                        std::cout << FG_GREEN << "(" << xPos << ", " << yPos << ") LinkCollection: " << collectionInfo.LinkCollection << RESET_COLOR << std::endl;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                free(deviceName);
                            }

                            // https://stackoverflow.com/a/27012730/8364403
                        }
                    }
                }
            }

            delete[] lpb;
        }
    }
}