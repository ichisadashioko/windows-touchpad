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
#include <map>

// Global variables:
HINSTANCE hInst; // current instance
BOOL isTouchpadRegistered            = FALSE;
static TCHAR szWindowClass[]         = _T("DesktopApp");                              // the main window class name
static TCHAR szTitle[]               = _T("Windows Desktop Guided Tour Application"); // the application's title bar
static BOOL showedConnectedTouchpads = FALSE;

struct COLLECTION_INFO {
    USHORT LinkCollection;
    RECT PhysicalRect;
};

// C doesn't have map or dictionary so we are going to use array of struct to replace that
struct HID_TOUCHPAD_INFO {
    TCHAR *Name                  = NULL;
    COLLECTION_INFO *Collections = NULL;
    unsigned int NumCollections  = 0;
};

static unsigned int numTouchpads    = 0;
static HID_TOUCHPAD_INFO *touchpads = NULL;

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

    if (!showedConnectedTouchpads) {
        showedConnectedTouchpads = TRUE;

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
                for (UINT i = 0; i < _numDevices; i++) {
                    RAWINPUTDEVICELIST rawInputDevice = rawInputDeviceList.get()[i];
                    if (rawInputDevice.dwType != RIM_TYPEHID) {
                        // skip other device types
                        continue;
                    }

                    std::cout << i << " - dwType: " << rawInputDevice.dwType << " - hDevice:" << rawInputDevice.hDevice << std::endl;

                    // x GetRawInputDeviceInfo (RIDI_DEVICENAME)
                    UINT deviceNameBufferSize = 0;
                    winReturnCode             = GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_DEVICENAME, NULL, &deviceNameBufferSize);
                    if (winReturnCode == (UINT)-1) {
                        std::cout << "Failed to call GetRawInputDeviceInfo for getting device name size at " << __FILE__ << ":" << __LINE__ << std::endl;
                        printLastError();
                    } else {
                        std::cout << "  deviceNameLength: " << deviceNameBufferSize << std::endl;
                        // the return value is the number of characters for the device name
                        // it is NOT the number of bytes for holding the device name
                        const UINT deviceNameLength = deviceNameBufferSize;

                        std::unique_ptr<TCHAR[]> deviceNameBuffer(new TCHAR[deviceNameLength + 1]);

                        // set string terminator
                        // if we don't do this, bad thing will happen
                        // the runtime will give all sort of exceptions that does not point to our code
                        deviceNameBuffer.get()[deviceNameLength] = 0;

                        winReturnCode = GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_DEVICENAME, deviceNameBuffer.get(), &deviceNameBufferSize);
                        if (winReturnCode == (UINT)-1) {
                            std::cout << "Failed to get device name at " << __FILE__ << ":" << __LINE__ << std::endl;
                            printLastError();
                        } else if (winReturnCode != deviceNameLength) {
                            std::cout << "GetRawInputDeviceInfo does not return the expected size " << winReturnCode << " (actual) vs " << deviceNameLength << " at " << __FILE__ << ":" << __LINE__ << std::endl;
                        } else {
                            TCHAR *deviceName = deviceNameBuffer.get();
                            std::cout << "  device name: ";
                            std::wcout << deviceName << std::endl;

                            // get preparsed data for HidP
                            UINT prepasedDataSize = 0;
                            winReturnCode         = GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_PREPARSEDDATA, NULL, &prepasedDataSize);
                            if (winReturnCode == (UINT)-1) {
                                std::cout << "Failed to call GetRawInputDeviceInfo to get prepased data size at " << __FILE__ ":" << __LINE__ << std::endl;
                                printLastError();
                            } else {
                                std::cout << "Prepased data size: " << prepasedDataSize << std::endl;
                                const UINT _prepasedDataSize = prepasedDataSize;
                                LPBYTE lpb                   = new BYTE[_prepasedDataSize];

                                if (lpb == NULL) {
                                    std::cout << "Failed to allocate memory at " << __FILE__ << ":" << __LINE__ << std::endl;
                                } else {
                                    winReturnCode = GetRawInputDeviceInfo(rawInputDevice.hDevice, RIDI_PREPARSEDDATA, lpb, &prepasedDataSize);
                                    if (winReturnCode == (UINT)-1) {
                                        std::cout << "Failed to call GetRawInputDeviceInfo to get prepased data at " << __FILE__ << ":" << __LINE__ << std::endl;
                                        printLastError();
                                    } else {
                                        PHIDP_PREPARSED_DATA preparsedData = (PHIDP_PREPARSED_DATA)lpb;

                                        NTSTATUS hidpReturnCode;

                                        // find HID capabilities
                                        HIDP_CAPS caps;
                                        hidpReturnCode = HidP_GetCaps(preparsedData, &caps);

                                        if (hidpReturnCode == HIDP_STATUS_SUCCESS) {
                                            // TODO continue parse raw input data

                                            USHORT numInputValueCaps        = caps.NumberInputValueCaps;
                                            const USHORT _numInputValueCaps = numInputValueCaps;

                                            // HIDP_VALUE_CAPS valueCaps;
                                            PHIDP_VALUE_CAPS valueCaps = new HIDP_VALUE_CAPS[numInputValueCaps];
                                            hidpReturnCode             = HidP_GetValueCaps(HidP_Input, valueCaps, &numInputValueCaps, preparsedData);

                                            if (hidpReturnCode == HIDP_STATUS_SUCCESS) {
                                                // x check if numInputValueCaps value has been changed
                                                std::cout << "NumberInputValueCaps: " << _numInputValueCaps << " (old) vs " << numInputValueCaps << " (new)" << std::endl;

                                                for (USHORT i = 0; i < _numInputValueCaps; i++) {
                                                    // TODO I don't know what are we doing here

                                                    HIDP_VALUE_CAPS cap = valueCaps[i];

                                                    if (cap.IsRange || !cap.IsAbsolute) {
                                                        continue;
                                                    }

                                                    if (cap.UsagePage == HID_USAGE_PAGE_GENERIC) {
                                                        std::cout << "=================================================" << std::endl;
                                                        std::cout << "LinkCollection: " << cap.LinkCollection << std::endl;

                                                        std::cout << FG_GREEN << "Finding device in global list..." << RESET_COLOR << std::endl;
                                                        unsigned int foundTouchpadIndex = (unsigned int)-1;

                                                        bool isTouchpadsNull              = (touchpads == NULL);
                                                        bool isTouchpadsRecordedSizeEmpty = (numTouchpads == 0);
                                                        if (isTouchpadsNull || isTouchpadsRecordedSizeEmpty) {
                                                            // the array/list/dictionary is empty
                                                            // allocate memory for the first entry
                                                            numTouchpads       = 1;
                                                            foundTouchpadIndex = 0;

                                                            // TODO recursive free pointers inside struct
                                                            // but we can't know the size of array pointer here
                                                            // because the size is recorded as 0

                                                            // shallow free memory in case if it has been assigned before
                                                            free(touchpads);
                                                            touchpads = (HID_TOUCHPAD_INFO *)malloc(sizeof(HID_TOUCHPAD_INFO));
                                                            if (touchpads == NULL) {
                                                                std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
                                                                exit(-1);
                                                            }

                                                            touchpads[foundTouchpadIndex].Name           = deviceName;
                                                            touchpads[foundTouchpadIndex].Collections    = NULL;
                                                            touchpads[foundTouchpadIndex].NumCollections = 0;
                                                        } else {
                                                            for (unsigned int touchpadIndex = 0; touchpadIndex < numTouchpads; touchpadIndex++) {
                                                                int compareNameResult = _tcscmp(deviceName, touchpads[touchpadIndex].Name);
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
                                                            foundTouchpadIndex = numTouchpads;
                                                            numTouchpads       = foundTouchpadIndex + 1;

                                                            // copy entries to new array
                                                            HID_TOUCHPAD_INFO *tmpTouchpadInfoArray = (HID_TOUCHPAD_INFO *)malloc(sizeof(HID_TOUCHPAD_INFO) * numTouchpads);
                                                            if (tmpTouchpadInfoArray == NULL) {
                                                                std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
                                                                exit(-1);
                                                            }

                                                            for (unsigned int touchpadIndex = 0; touchpadIndex < foundTouchpadIndex; touchpadIndex++) {
                                                                tmpTouchpadInfoArray[touchpadIndex] = touchpads[touchpadIndex];
                                                            }

                                                            free(touchpads);
                                                            touchpads                          = tmpTouchpadInfoArray;
                                                            touchpads[foundTouchpadIndex].Name = deviceName;
                                                        }

                                                        std::cout << FG_GREEN << "foundTouchpadIndex: " << RESET_COLOR << foundTouchpadIndex << std::endl;

                                                        std::cout << FG_BRIGHT_BLUE << "found device name:   " << RESET_COLOR;
                                                        std::wcout << touchpads[foundTouchpadIndex].Name << std::endl;

                                                        std::cout << FG_BRIGHT_RED << "current device name: " << RESET_COLOR;
                                                        std::wcout << deviceName << std::endl;

                                                        long findLinkCollectionStartTime = clock();

                                                        unsigned int foundLinkCollectionIndex = (unsigned int)-1;
                                                        bool isCollectionNull                 = (touchpads[foundTouchpadIndex].Collections == NULL);
                                                        bool isCollectionRecordedSizeEmpty    = (touchpads[foundTouchpadIndex].NumCollections == 0);

                                                        if (isCollectionNull || isCollectionRecordedSizeEmpty) {
                                                            foundLinkCollectionIndex                     = 0;
                                                            touchpads[foundTouchpadIndex].NumCollections = (unsigned int)1;

                                                            free(touchpads[foundTouchpadIndex].Collections);
                                                            touchpads[foundTouchpadIndex].Collections = (COLLECTION_INFO *)malloc(sizeof(COLLECTION_INFO));

                                                            if (touchpads[foundTouchpadIndex].Collections == NULL) {
                                                                std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
                                                                exit(-1);
                                                            }

                                                            touchpads[foundTouchpadIndex].Collections[0].LinkCollection = cap.LinkCollection;
                                                        } else {
                                                            std::cout << BG_BLUE << "NumCollections: " << touchpads[foundTouchpadIndex].NumCollections << RESET_COLOR << std::endl;
                                                            for (unsigned int linkCollectionIndex = 0; linkCollectionIndex < touchpads[foundTouchpadIndex].NumCollections; linkCollectionIndex++) {
                                                                if (touchpads[foundTouchpadIndex].Collections[linkCollectionIndex].LinkCollection == cap.LinkCollection) {
                                                                    foundLinkCollectionIndex = linkCollectionIndex;
                                                                    break;
                                                                }
                                                            }
                                                        }

                                                        if (foundLinkCollectionIndex == (unsigned int)-1) {
                                                            foundLinkCollectionIndex                     = touchpads[foundTouchpadIndex].NumCollections;
                                                            touchpads[foundTouchpadIndex].NumCollections = foundLinkCollectionIndex + 1;

                                                            COLLECTION_INFO *tmpCollectionArray = (COLLECTION_INFO *)malloc(sizeof(COLLECTION_INFO) * touchpads[foundTouchpadIndex].NumCollections);
                                                            if (tmpCollectionArray == NULL) {
                                                                std::cout << FG_RED << "malloc failed at " << __FILE__ << ":" << __LINE__ << RESET_COLOR << std::endl;
                                                                exit(-1);
                                                            }

                                                            for (unsigned int linkCollectionIndex = 0; linkCollectionIndex < foundLinkCollectionIndex; linkCollectionIndex++) {
                                                                tmpCollectionArray[linkCollectionIndex] = touchpads[foundTouchpadIndex].Collections[linkCollectionIndex];
                                                            }

                                                            free(touchpads[foundTouchpadIndex].Collections);
                                                            touchpads[foundTouchpadIndex].Collections                                          = tmpCollectionArray;
                                                            touchpads[foundTouchpadIndex].Collections[foundLinkCollectionIndex].LinkCollection = cap.LinkCollection;
                                                        }

                                                        long findLinkCollectionEndTime = clock();
                                                        long findLinkCollectionTime    = findLinkCollectionEndTime - findLinkCollectionStartTime;
                                                        std::cout << FG_GREEN << "findLinkCollectionTime: " << RESET_COLOR << findLinkCollectionTime << std::endl;

                                                        if (cap.NotRange.Usage == HID_USAGE_GENERIC_X) {
                                                            touchpads[foundTouchpadIndex].Collections[foundLinkCollectionIndex].PhysicalRect.left  = cap.PhysicalMin;
                                                            touchpads[foundTouchpadIndex].Collections[foundLinkCollectionIndex].PhysicalRect.right = cap.PhysicalMax;
                                                            std::cout << "  Left: " << cap.PhysicalMin << std::endl;
                                                            std::cout << "  Right: " << cap.PhysicalMax << std::endl;
                                                        } else if (cap.NotRange.Usage == HID_USAGE_GENERIC_Y) {
                                                            touchpads[foundTouchpadIndex].Collections[foundLinkCollectionIndex].PhysicalRect.top    = cap.PhysicalMin;
                                                            touchpads[foundTouchpadIndex].Collections[foundLinkCollectionIndex].PhysicalRect.bottom = cap.PhysicalMax;
                                                            std::cout << "  Top: " << cap.PhysicalMin << std::endl;
                                                            std::cout << "  Bottom: " << cap.PhysicalMax << std::endl;
                                                        }
                                                    }
                                                }
                                            } else if (hidpReturnCode == HIDP_STATUS_INVALID_PREPARSED_DATA) {
                                                std::cout << "The specified prepased data is invalid at " << __FILE__ << ":" << __LINE__ << std::endl;
                                            } else {
                                                std::cout << "Failed to call HidP_GetValueCaps! Unknown status code: " << hidpReturnCode << " at " << __FILE__ << ":" << __LINE__ << std::endl;
                                            }

                                            delete[] valueCaps;
                                        } else if (hidpReturnCode == HIDP_STATUS_INVALID_PREPARSED_DATA) {
                                            std::cout << "The specified prepased data is invalid at " << __FILE__ << ":" << __LINE__ << std::endl;
                                        } else {
                                            std::cout << "Failed to call HidP_GetCaps! Unknown status code: " << hidpReturnCode << " at " << __FILE__ << ":" << __LINE__ << std::endl;
                                        }
                                    }
                                }

                                delete[] lpb;
                            }
                        }
                    }
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

                            // https://stackoverflow.com/a/27012730/8364403
                            // ULONG numTouchContacts;
                            // NTSTATUS hidpReturnCode = HidP_GetUsageValue(
                            //  HidP_Input, // ReportType
                            //  HID_USAGE_PAGE_DIGITIZER, // UsagePage
                            //  // LinkCollection
                            //);
                        }
                    }
                }
            }

            delete[] lpb;
        }
    }
}