#include <Windows.h>

#include <WinUser.h>
#pragma comment(lib, "User32.lib")

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "termcolor.h"
#include "utils.h"
#include "touchpad.h"
#include "touchevents.h"
#include "point2d.h"
#include "stroke.h"

#define LOG_EVERY_INPUT_MESSAGES
#undef LOG_EVERY_INPUT_MESSAGES

static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[]       = _T("F3: start writing - ESC: stop writing - C: clear - Q: close the application");

#define VK_C_KEY 0x43;
#define VK_Q_KEY 0x51;

struct ApplicationState
{
  HID_DEVICE_INFO_LIST device_info_list;
  TOUCH_DATA_LIST previous_touches;
  StrokeList strokes;
  ULONG tracking_touch_id;
  // flag to toggle drawing state
  int is_drawing;

  // https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
  // TODO add option to change the key bindings
  // TODO add option to add multiple key bindings (array of key bindings for each actions)

  int turn_off_drawing_key_code;
  int turn_on_drawing_key_code;
  int quit_application_key_code;
  int clear_drawing_canvas_key_code;
  int call_block_input_flag;
  int call_unblock_input_flag;
};

typedef struct ApplicationState ApplicationState;

static ApplicationState* g_app_state = NULL;

void mParseConnectedInputDevices()
{
  printf(FG_BLUE);
  printf("Parsing all HID devices...\n");
  printf(RESET_COLOR);

  // find number of connected devices

  UINT numDevices;
  RAWINPUTDEVICELIST* rawInputDeviceList = NULL;

  mGetRawInputDeviceList(&numDevices, &rawInputDeviceList);

  printf("Number of raw input devices: %d\n", numDevices);
  for (UINT deviceIndex = 0; deviceIndex < numDevices; deviceIndex++)
  {
    printf(BG_GREEN);
    printf("===== Device #%d =====\n", deviceIndex);
    printf(RESET_COLOR);
    RAWINPUTDEVICELIST rawInputDevice = rawInputDeviceList[deviceIndex];
    if (rawInputDevice.dwType != RIM_TYPEHID)
    {
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
    if (hidpReturnCode != HIDP_STATUS_SUCCESS)
    {
      print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
      exit(-1);
    }

    printf("NumberInputValueCaps: %d\n", caps.NumberInputValueCaps);
    printf("NumberInputButtonCaps: %d\n", caps.NumberInputButtonCaps);

    int isButtonCapsEmpty = (caps.NumberInputButtonCaps == 0);

    if (!isButtonCapsEmpty)
    {
      UINT deviceNameLength;
      TCHAR* deviceName = NULL;
      unsigned int cbDeviceName;

      mGetRawInputDeviceName(rawInputDevice.hDevice, &deviceName, &deviceNameLength, &cbDeviceName);

      printf("Device name: ");
      wprintf(deviceName);
      printf("\n");

      printf(FG_GREEN);
      printf("Finding device in global list...\n");
      printf(RESET_COLOR);
      unsigned int foundHidIdx;
      int returnCode = FindInputDeviceInList(&(g_app_state->device_info_list), deviceName, cbDeviceName, preparsedData, cbDataSize, &foundHidIdx);
      if (returnCode != 0)
      {
        printf("FindInputDeviceInList failed at %s:%d\n", __FILE__, __LINE__);
        exit(-1);
      }

      printf(FG_GREEN);
      printf("foundHIDIdx: %d\n", foundHidIdx);
      printf(RESET_COLOR);

      printf(FG_BRIGHT_BLUE);
      printf("found device name: ");
      wprintf(g_app_state->device_info_list.Entries[foundHidIdx].Name);
      printf("\n");
      printf(RESET_COLOR);

      printf(FG_BRIGHT_BLUE);
      printf("current device name: ");
      wprintf(deviceName);
      printf("\n");
      printf(RESET_COLOR);

      if (caps.NumberInputValueCaps != 0)
      {
        const USHORT numValueCaps = caps.NumberInputValueCaps;
        USHORT _numValueCaps      = numValueCaps;

        PHIDP_VALUE_CAPS valueCaps = (PHIDP_VALUE_CAPS)mMalloc(sizeof(HIDP_VALUE_CAPS) * numValueCaps, __FILE__, __LINE__);

        hidpReturnCode = HidP_GetValueCaps(HidP_Input, valueCaps, &_numValueCaps, preparsedData);
        if (hidpReturnCode != HIDP_STATUS_SUCCESS)
        {
          print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
        }

        // x check if numValueCaps value has been changed
        printf("NumberInputValueCaps: %d (old) vs %d (new)\n", numValueCaps, _numValueCaps);

        for (USHORT valueCapIndex = 0; valueCapIndex < numValueCaps; valueCapIndex++)
        {
          HIDP_VALUE_CAPS cap = valueCaps[valueCapIndex];

          if (cap.IsRange || !cap.IsAbsolute)
          {
            continue;
          }

          unsigned int foundLinkColIdx;
          int returnCode = FindLinkCollectionInList(&(g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList), cap.LinkCollection, &foundLinkColIdx);
          if (returnCode != 0)
          {
            printf("FindLinkCollectionInList failed at %s:%d\n", __FILE__, __LINE__);
            exit(-1);
          }

          printf(FG_GREEN);
          printf("[ValueCaps] foundLinkCollectionIndex: %d\n", foundLinkColIdx);
          printf(RESET_COLOR);

          if (cap.UsagePage == HID_USAGE_PAGE_GENERIC)
          {
            printf("=====================================================\n");
            printf("LinkCollection: %d\n", cap.LinkCollection);

            if (cap.NotRange.Usage == HID_USAGE_GENERIC_X)
            {
              g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].HasX               = 1;
              g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].PhysicalRect.left  = cap.PhysicalMin;
              g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].PhysicalRect.right = cap.PhysicalMax;
              printf("  Left: %d\n", cap.PhysicalMin);
              printf("  Right: %d\n", cap.PhysicalMax);
            }
            else if (cap.NotRange.Usage == HID_USAGE_GENERIC_Y)
            {
              g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].HasY                = 1;
              g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].PhysicalRect.top    = cap.PhysicalMin;
              g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].PhysicalRect.bottom = cap.PhysicalMax;
              printf("  Top: %d\n", cap.PhysicalMin);
              printf("  Bottom: %d\n", cap.PhysicalMax);
            }
          }
          else if (cap.UsagePage == HID_USAGE_PAGE_DIGITIZER)
          {
            if (cap.NotRange.Usage == HID_USAGE_DIGITIZER_CONTACT_ID)
            {
              g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].HasContactID = 1;
            }
            else if (cap.NotRange.Usage == HID_USAGE_DIGITIZER_CONTACT_COUNT)
            {
              g_app_state->device_info_list.Entries[foundHidIdx].ContactCountLinkCollection = cap.LinkCollection;
            }
          }
        }

        free(valueCaps);
      }

      if (caps.NumberInputButtonCaps != 0)
      {
        const USHORT numButtonCaps = caps.NumberInputButtonCaps;
        USHORT _numButtonCaps      = numButtonCaps;

        PHIDP_BUTTON_CAPS buttonCaps = (PHIDP_BUTTON_CAPS)mMalloc(sizeof(HIDP_BUTTON_CAPS) * numButtonCaps, __FILE__, __LINE__);

        hidpReturnCode = HidP_GetButtonCaps(HidP_Input, buttonCaps, &_numButtonCaps, preparsedData);
        if (hidpReturnCode != HIDP_STATUS_SUCCESS)
        {
          print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
          exit(-1);
        }

        for (USHORT buttonCapIndex = 0; buttonCapIndex < numButtonCaps; buttonCapIndex++)
        {
          HIDP_BUTTON_CAPS buttonCap = buttonCaps[buttonCapIndex];

          if (buttonCap.IsRange)
          {
            continue;
          }

          printf(FG_BLUE);
          printf("[ButtonCaps] Index: %d, UsagePage: %d, Usage: %d, DIGITIZER: %d, IsRange: %d\n", buttonCapIndex, buttonCap.UsagePage, buttonCap.NotRange.Usage, buttonCap.UsagePage, buttonCap.IsRange);
          printf(RESET_COLOR);

          if (buttonCap.UsagePage == HID_USAGE_PAGE_DIGITIZER)
          {
            if (buttonCap.NotRange.Usage == HID_USAGE_DIGITIZER_TIP_SWITCH)
            {
              unsigned int foundLinkColIdx;
              int returnCode = FindLinkCollectionInList(&(g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList), buttonCap.LinkCollection, &foundLinkColIdx);

              printf(FG_GREEN);
              printf("[ButtonCaps] foundLinkCollectionIndex: %d\n", foundLinkColIdx);
              printf(RESET_COLOR);

              g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries[foundLinkColIdx].HasTipSwitch = 1;
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

void mRegisterRawInput(HWND hwnd)
{
  // register Windows Precision Touchpad top-level HID collection
  RAWINPUTDEVICE rid;
  clock_t ts = clock();

  rid.usUsagePage = HID_USAGE_PAGE_DIGITIZER;
  rid.usUsage     = HID_USAGE_DIGITIZER_TOUCH_PAD;
  rid.dwFlags     = RIDEV_INPUTSINK;
  rid.hwndTarget  = hwnd;

  if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
  {
    printf(FG_GREEN);
    printf("[%d] Successfully register touchpad!\n", ts);
    printf(RESET_COLOR);
  }
  else
  {
    printf(FG_RED);
    printf("[%d] Failed to register touchpad at %s:%d\n", ts, __FILE__, __LINE__);
    printf(RESET_COLOR);
    mGetLastError();
    exit(-1);
  }
}

void mHandleCreateMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  mRegisterRawInput(hwnd);
}

void mHandleInputMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  clock_t ts = clock();

  // following guide: https://docs.microsoft.com/en-us/windows/win32/inputdev/using-raw-input#performing-a-standard-read-of-raw-input

  // Get the size of RAWINPUT by calling GetRawInputData() with pData = NULL

  if (g_app_state->is_drawing != 0)
  {
    UINT rawInputSize;
    PRAWINPUT rawInputData = NULL;

    mGetRawInputData((HRAWINPUT)lParam, &rawInputSize, (LPVOID*)(&rawInputData));

    // Parse the RAWINPUT data.
    if (rawInputData->header.dwType == RIM_TYPEHID)
    {
      // TODO what does `dwCount` represent?
      DWORD count   = rawInputData->data.hid.dwCount;
      BYTE* rawData = rawInputData->data.hid.bRawData;

      if (count != 0)
      {
        UINT deviceNameLength;
        TCHAR* deviceName = NULL;
        unsigned int cbDeviceName;

        mGetRawInputDeviceName(rawInputData->header.hDevice, &deviceName, &deviceNameLength, &cbDeviceName);

        unsigned int foundHidIdx = (unsigned int)-1;

        int isTouchpadsNull              = (g_app_state->device_info_list.Entries == NULL);
        int isTouchpadsRecordedSizeEmpty = (g_app_state->device_info_list.Size == 0);
        if (isTouchpadsNull || isTouchpadsRecordedSizeEmpty)
        {
          // TODO parse new connected device data
        }
        else
        {
          for (unsigned int touchpadIndex = 0; touchpadIndex < g_app_state->device_info_list.Size; touchpadIndex++)
          {
            int compareNameResult = _tcscmp(deviceName, g_app_state->device_info_list.Entries[touchpadIndex].Name);
            if (compareNameResult == 0)
            {
              foundHidIdx = touchpadIndex;
              break;
            }
          }
        }

        if (foundHidIdx == (unsigned int)-1)
        {
          // TODO parse new connected device data
        }
        else
        {
          int isLinkColArrayNull  = (g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries == NULL);
          int isLinkColArrayEmpty = (g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Size == 0);
          int isPreparsedDataNull = (g_app_state->device_info_list.Entries[foundHidIdx].PreparedData == NULL);

          if (isLinkColArrayNull || isLinkColArrayEmpty)
          {
            printf(FG_RED);
            printf("Cannot find any LinkCollection(s). Try parse the PREPARED_DATA may help. TODO\n");
            printf(RESET_COLOR);
          }
          else if (isPreparsedDataNull)
          {
            printf(FG_RED);
            printf("Cannot find PreparsedData at %s:%d\n", __FILE__, __LINE__);
            printf(RESET_COLOR);
          }
          else
          {
#ifdef LOG_EVERY_INPUT_MESSAGES
            printf("[%d]", ts);
#endif
            NTSTATUS hidpReturnCode;
            ULONG usageValue;

            PHIDP_PREPARSED_DATA preparsedHIDData = g_app_state->device_info_list.Entries[foundHidIdx].PreparedData;

            if (g_app_state->device_info_list.Entries[foundHidIdx].ContactCountLinkCollection == (USHORT)-1)
            {
              printf(FG_RED);
              printf("Cannot find contact count Link Collection!\n");
              printf(RESET_COLOR);
            }
            else
            {
              hidpReturnCode = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_DIGITIZER, g_app_state->device_info_list.Entries[foundHidIdx].ContactCountLinkCollection, HID_USAGE_DIGITIZER_CONTACT_COUNT, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

              if (hidpReturnCode != HIDP_STATUS_SUCCESS)
              {
                printf(FG_RED);
                printf("Failed to read number of contacts!\n");
                printf(RESET_COLOR);
                print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                exit(-1);
              }

              ULONG numContacts = usageValue;

#ifdef LOG_EVERY_INPUT_MESSAGES
              printf(FG_BRIGHT_BLUE);
              printf("numContacts: %d\n", numContacts);
              printf(RESET_COLOR);
#endif

              if (numContacts > g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Size)
              {
                // TODO how should we deal with this edge case
                printf(FG_RED);
                printf("number of contacts is greater than Link Collection Array size at %s:%d\n", __FILE__, __LINE__);
                printf(RESET_COLOR);
                exit(-1);
              }
              else
              {
                for (unsigned int linkColIdx = 0; linkColIdx < numContacts; linkColIdx++)
                {
                  HID_TOUCH_LINK_COL_INFO collectionInfo = g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries[linkColIdx];

                  if (collectionInfo.HasX && collectionInfo.HasY && collectionInfo.HasContactID && collectionInfo.HasTipSwitch)
                  {
                    hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.LinkColID, 0x30, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

                    if (hidpReturnCode != HIDP_STATUS_SUCCESS)
                    {
                      printf(FG_RED);
                      printf("Failed to read x position!\n");
                      printf(RESET_COLOR);
                      print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                      exit(-1);
                    }

                    ULONG xPos = usageValue;

                    hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.LinkColID, 0x31, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
                    if (hidpReturnCode != HIDP_STATUS_SUCCESS)
                    {
                      printf(FG_RED);
                      printf("Failed to read y position!\n");
                      printf(RESET_COLOR);
                      print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                      exit(-1);
                    }

                    ULONG yPos = usageValue;

                    hidpReturnCode = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.LinkColID, HID_USAGE_DIGITIZER_CONTACT_ID, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
                    if (hidpReturnCode != HIDP_STATUS_SUCCESS)
                    {
                      printf(FG_RED);
                      printf("Failed to read touch ID!\n");
                      printf(RESET_COLOR);
                      print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                      exit(-1);
                    }

                    ULONG touchId = usageValue;

                    const ULONG maxNumButtons = HidP_MaxUsageListLength(HidP_Input, HID_USAGE_PAGE_DIGITIZER, preparsedHIDData);

                    ULONG _maxNumButtons = maxNumButtons;

                    USAGE* buttonUsageArray = (USAGE*)mMalloc(sizeof(USAGE) * maxNumButtons, __FILE__, __LINE__);

                    hidpReturnCode = HidP_GetUsages(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.LinkColID, buttonUsageArray, &_maxNumButtons, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

                    if (hidpReturnCode != HIDP_STATUS_SUCCESS)
                    {
                      printf(FG_RED);
                      printf("HidP_GetUsages failed!\n");
                      printf(RESET_COLOR);
                      print_HidP_errors(hidpReturnCode, __FILE__, __LINE__);
                      exit(-1);
                    }

                    int isContactOnSurface = 0;

                    for (ULONG usageIdx = 0; usageIdx < maxNumButtons; usageIdx++)
                    {
                      if (buttonUsageArray[usageIdx] == HID_USAGE_DIGITIZER_TIP_SWITCH)
                      {
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
                    int cStyleFunctionReturnCode = mInterpretRawTouchInput(&g_app_state->previous_touches, curTouch, &touchType);
                    if (cStyleFunctionReturnCode != 0)
                    {
                      printf(FG_RED);
                      printf("mInterpretRawTouchInput failed at %s:%d\n", __FILE__, __LINE__);
                      printf(RESET_COLOR);
                      exit(-1);
                    }

                    Point2D touchPos = (Point2D){.X = curTouch.X, .Y = curTouch.Y};

                    if (g_app_state->tracking_touch_id == (ULONG)-1)
                    {
                      if (touchType == EVENT_TYPE_TOUCH_DOWN)
                      {
                        g_app_state->tracking_touch_id = curTouch.TouchID;
                        // TODO create new stroke
                        // TODO check return value for indication of errors
                        mCreateNewStroke(touchPos, &g_app_state->strokes);
                      }
                      else
                      {
                        // wait for touch down event to register new stroke
                      }
                    }
                    else if (curTouch.TouchID == g_app_state->tracking_touch_id)
                    {
                      if (touchType == EVENT_TYPE_TOUCH_MOVE)
                      {
                        // we skip EVENT_TYPE_TOUCH_MOVE_UNCHANGED here
                        // TODO append touch position to the last stroke
                        if ((g_app_state->strokes.Entries == NULL) || (g_app_state->strokes.Size == 0))
                        {
                          printf(FG_RED);
                          printf("The application state is broken!\n");
                          printf(RESET_COLOR);
                          exit(-1);
                        }
                        else
                        {
                          // TODO check return value for indication of errors
                          mAppendPoint2DToList(touchPos, &g_app_state->strokes.Entries[g_app_state->strokes.Size - 1]);

                          Point2DList stroke = g_app_state->strokes.Entries[g_app_state->strokes.Size - 1];
                          if (stroke.Size < 2)
                          {
                            printf(FG_RED);
                            printf("The application state is broken!\n");
                            printf(RESET_COLOR);
                            exit(-1);
                          }
                          else
                          {
                            HDC hdc        = GetDC(hwnd);
                            HPEN strokePen = CreatePen(PS_SOLID, 20, RGB(255, 255, 255));
                            SelectObject(hdc, strokePen);
                            MoveToEx(hdc, (int)stroke.Entries[stroke.Size - 2].X, (int)stroke.Entries[stroke.Size - 2].Y, (LPPOINT)NULL);
                            LineTo(hdc, (int)stroke.Entries[stroke.Size - 1].X, (int)stroke.Entries[stroke.Size - 1].Y);
                            ReleaseDC(hwnd, hdc);
                          }
                        }
                      }
                      else if (touchType == EVENT_TYPE_TOUCH_UP)
                      {
                        // I sure that the touch position is the same with the last touch position
                        g_app_state->tracking_touch_id = (ULONG)-1;
                      }
                    }

#ifdef LOG_EVERY_INPUT_MESSAGES
                    const char* touchTypeStr;

                    if (touchType == EVENT_TYPE_TOUCH_UP)
                    {
                      touchTypeStr = "touch up";
                    }
                    else if (touchType == EVENT_TYPE_TOUCH_MOVE)
                    {
                      touchTypeStr = "touch move";
                    }
                    else if (touchType == EVENT_TYPE_TOUCH_DOWN)
                    {
                      touchTypeStr = "touch down";
                    }
                    else if (touchType == EVENT_TYPE_TOUCH_MOVE_UNCHANGED)
                    {
                      touchTypeStr = "touch move unchanged";
                    }
                    else
                    {
                      printf(FG_RED);
                      printf("unknown event type: %d\n", touchType);
                      printf(RESET_COLOR);
                      exit(-1);
                    }

                    printf(FG_GREEN);
                    printf("LinkColId: %d, touchID: %d, tipSwitch: %d, position: (%d, %d), eventType: %s\n", collectionInfo.LinkColID, touchId, isContactOnSurface, xPos, yPos, touchTypeStr);
                    printf(RESET_COLOR);
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
}

void mHandleResizeMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  InvalidateRect(hwnd, NULL, FALSE);
}

void mHandlePaintMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  // clock_t ts = clock();
  HDC hdc;
  PAINTSTRUCT ps;
  RECT rc;

  GetClientRect(hwnd, &rc);

  if (rc.bottom == 0)
  {
    return;
  }

  hdc = BeginPaint(hwnd, &ps);

  // draw black background
  HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
  FillRect(hdc, &rc, bgBrush);

  // TODO change color for every strokes
  HPEN strokePen = CreatePen(PS_SOLID, 20, RGB(255, 255, 255));
  SelectObject(hdc, strokePen);

  if ((g_app_state->strokes.Entries != NULL) && (g_app_state->strokes.Size != 0))
  {
    for (unsigned int strokeIdx = 0; strokeIdx < g_app_state->strokes.Size; strokeIdx++)
    {
      // TODO change rendering color for each strokes
      Point2DList strokeData = g_app_state->strokes.Entries[strokeIdx];
      if (strokeData.Size < 2)
      {
        // TODO improve rendering strokes logic
        continue;
      }
      else
      {
        MoveToEx(hdc, (int)strokeData.Entries[0].X, (int)strokeData.Entries[0].Y, (LPPOINT)NULL);

        for (unsigned int pointIdx = 1; pointIdx < strokeData.Size; pointIdx++)
        {
          LineTo(hdc, (int)strokeData.Entries[pointIdx].X, (int)strokeData.Entries[pointIdx].Y);
        }
      }
    }
  }

  EndPaint(hwnd, &ps);
}

void mHandleKeyUpMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  clock_t ts = clock();

  printf(FG_GREEN);
  printf("[%d] WM_KEYUP: 0x%x\n", ts, wParam);
  printf(RESET_COLOR);

  int virtual_key_code = (int)wParam;
  if (virtual_key_code == g_app_state->turn_off_drawing_key_code)
  {
    g_app_state->is_drawing = 0;

    g_app_state->call_unblock_input_flag = 1;
  }
  else if (virtual_key_code == g_app_state->turn_on_drawing_key_code)
  {
    g_app_state->is_drawing = 1;

    g_app_state->call_block_input_flag = 1;
  }
  else if (virtual_key_code == g_app_state->clear_drawing_canvas_key_code)
  {
    g_app_state->tracking_touch_id = (ULONG)-1;

    if (g_app_state->previous_touches.Entries != NULL)
    {
      free(g_app_state->previous_touches.Entries);
      g_app_state->previous_touches.Entries = NULL;
      g_app_state->previous_touches.Size    = 0;
    }

    if (g_app_state->strokes.Entries != NULL)
    {
      for (unsigned int strokeIdx = 0; strokeIdx < g_app_state->strokes.Size; strokeIdx++)
      {
        Point2DList stroke = g_app_state->strokes.Entries[strokeIdx];
        if (stroke.Entries != NULL)
        {
          free(stroke.Entries);
          stroke.Entries = NULL;
          stroke.Size    = 0;
        }
      }

      free(g_app_state->strokes.Entries);
      g_app_state->strokes.Entries = NULL;
      g_app_state->strokes.Size    = 0;
    }

    InvalidateRect(hwnd, NULL, FALSE);
  }
  else if (virtual_key_code == g_app_state->quit_application_key_code)
  {
    PostQuitMessage(0);
  }
}

LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  clock_t ts = clock();

  switch (uMsg)
  {
    case WM_CREATE:
    {
      mHandleCreateMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_INPUT:
    {
      mHandleInputMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_PAINT:
    {
      mHandlePaintMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_SIZE:
    {
      mHandleResizeMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_KEYUP:
    {
      mHandleKeyUpMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_SYSKEYUP:
    {
      printf(FG_GREEN);
      printf("[%d] WM_SYSKEYUP: 0x%x\n", ts, wParam);
      printf(RESET_COLOR);
      break;
    }
    case WM_DESTROY:
    {
      PostQuitMessage(0);
      break;
    }
    default:
    {
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
  }

  return 0;
}

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  mParseConnectedInputDevices();

  // TODO detect and prompt user to select touchpad device and parse its width and height
  // default values
  int nWidth  = 720;
  int nHeight = 480;

  if ((g_app_state->device_info_list.Entries != NULL) && (g_app_state->device_info_list.Size != 0))
  {
    // TODO check for valid touchpad device
    HID_DEVICE_INFO inputDevice = g_app_state->device_info_list.Entries[0];

    if ((inputDevice.LinkColInfoList.Entries == NULL) || (inputDevice.LinkColInfoList.Size == 0))
    {
      printf(FG_RED);
      printf("Failed to parse link collections of input device!\n");
      printf(RESET_COLOR);
      return -1;
    }
    else
    {
      for (unsigned int linkColIdx = 0; linkColIdx < inputDevice.LinkColInfoList.Size; linkColIdx++)
      {
        HID_TOUCH_LINK_COL_INFO linkCollectionInfo = inputDevice.LinkColInfoList.Entries[linkColIdx];
        if (linkCollectionInfo.HasX && linkCollectionInfo.HasY)
        {
          // TODO Should we need to parse every single touch link collections? For now, I think one is sufficient.
          // TODO validate values (e.g. 0 or > screen size)
          nWidth  = linkCollectionInfo.PhysicalRect.right;
          nHeight = linkCollectionInfo.PhysicalRect.bottom;
          break;
        }
      }
    }
  }
  else
  {
    printf(FG_RED);
    printf("Failed to parse input devices!\n");
    printf(RESET_COLOR);
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

  if (!RegisterClassEx(&wcex))
  {
    printf("RegisterClassEx filed at %s:%d\n", __FILE__, __LINE__);
    mGetLastError();
    return -1;
  }

  HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_EX_LAYERED, CW_USEDEFAULT, CW_USEDEFAULT, nWidth, nHeight, NULL, NULL, hInstance, NULL);

  if (!hwnd)
  {
    printf("CreateWindow filed at %s:%d\n", __FILE__, __LINE__);
    mGetLastError();
    return -1;
  }

  // make the window transparent
  SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
  SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  // BOOL block_input_retval;
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    if (g_app_state->call_block_input_flag != 0)
    {
      // TODO find a way to call BlockInput without the need of UAC permission as KeyFreeze
      // block_input_retval = BlockInput(TRUE);
      // printf(FG_YELLOW);
      // printf("BlockInput(TRUE): %d\n", block_input_retval);
      // printf(RESET_COLOR);
      // if (block_input_retval == 0)
      //{
      //  mGetLastError();
      //}

      RECT lockCursorRect = (RECT){.left = 0, .top = 400, .right = 1, .bottom = 401};
      ClipCursor(&lockCursorRect);

      g_app_state->call_block_input_flag = 0;
    }
    else if (g_app_state->call_unblock_input_flag != 0)
    {
      // TODO find a way to detect keyboard state while blocking input
      // block_input_retval = BlockInput(FALSE);
      // printf(FG_YELLOW);
      // printf("BlockInput(FALSE): %d\n", block_input_retval);
      // printf(RESET_COLOR);
      // if (block_input_retval == 0)
      //{
      //  mGetLastError();
      //}

      ClipCursor(NULL);
      g_app_state->call_unblock_input_flag = 0;
    }
  }

  return (int)msg.wParam;
}

int main()
{
#define TEST
#undef TEST
#ifdef TEST
  test_mAppendPoint2DToList();
  return 0;
#endif

  g_app_state = (ApplicationState*)mMalloc(sizeof(ApplicationState), __FILE__, __LINE__);

  g_app_state->device_info_list  = (HID_DEVICE_INFO_LIST){.Entries = NULL, .Size = 0};
  g_app_state->previous_touches  = (TOUCH_DATA_LIST){.Entries = NULL, .Size = 0};
  g_app_state->strokes           = (StrokeList){.Entries = NULL, .Size = 0};
  g_app_state->tracking_touch_id = -1;
  g_app_state->is_drawing        = 0;

  g_app_state->turn_off_drawing_key_code     = VK_ESCAPE;
  g_app_state->turn_on_drawing_key_code      = VK_F3;
  g_app_state->quit_application_key_code     = VK_Q_KEY;
  g_app_state->clear_drawing_canvas_key_code = VK_C_KEY;
  g_app_state->call_block_input_flag         = 0;
  g_app_state->call_unblock_input_flag       = 0;

  return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
};
