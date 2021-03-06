#pragma once
#include <Windows.h>
#include <sal.h>
#include <tchar.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "termcolor.h"
#include "kankaku_utils.h"
#include "kankaku_touchpad.h"
#include "kankaku_touchevents.h"
#include "kankaku_point2d.h"
#include "kankaku_stroke.h"
#include "kankaku_serialization.h"

static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[]       = _T("F3: start writing - ESC: stop writing - C: clear - Q: close the application");

// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
#define VK_C_KEY 0x43;
#define VK_Q_KEY 0x51;
#define VK_S_KEY 0x53;

struct ApplicationState
{
  HID_DEVICE_INFO_LIST device_info_list;
};

typedef struct ApplicationState ApplicationState;

static ApplicationState* g_app_state = NULL;

void main_parse_connected_input_devices()
{
  printf("%sParsing all HID devices...%s\n", FG_BRIGHT_BLUE, RESET_COLOR);

  // find number of connected devices

  UINT numDevices;
  RAWINPUTDEVICELIST* rawInputDeviceList = NULL;

  kankaku_touchpad_get_raw_input_device_list(&numDevices, &rawInputDeviceList);

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

    kankaku_touchpad_get_raw_input_device_preparsed_data(rawInputDevice.hDevice, &preparsedData, &cbDataSize);

    NTSTATUS hidpReturnCode;

    // find HID capabilities
    HIDP_CAPS caps;
    hidpReturnCode = HidP_GetCaps(preparsedData, &caps);
    if (hidpReturnCode != HIDP_STATUS_SUCCESS)
    {
      utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
      exit(-1);
    }

    printf("NumberInputValueCaps: %d\n", caps.NumberInputValueCaps);
    printf("NumberInputButtonCaps: %d\n", caps.NumberInputButtonCaps);

    int isButtonCapsEmpty = (caps.NumberInputButtonCaps == 0);

    if (!isButtonCapsEmpty)
    {
      UINT deviceNameLength;
      char* deviceName = NULL;
      size_t cbDeviceName;

      kankaku_touchpad_get_raw_input_device_name(rawInputDevice.hDevice, &deviceName, &deviceNameLength, &cbDeviceName);

      unsigned int foundHidIdx;
      int returnCode = utils_find_input_device_index_by_name(&(g_app_state->device_info_list), deviceName, cbDeviceName, preparsedData, cbDataSize, &foundHidIdx);
      if (returnCode != 0)
      {
        fprintf(stderr, "%sutils_find_input_device_index_by_name failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
        exit(-1);
      }

      if (caps.NumberInputValueCaps != 0)
      {
        const USHORT numValueCaps = caps.NumberInputValueCaps;
        USHORT _numValueCaps      = numValueCaps;

        PHIDP_VALUE_CAPS valueCaps = (PHIDP_VALUE_CAPS)kankaku_utils_malloc_or_die(sizeof(HIDP_VALUE_CAPS) * numValueCaps, __FILE__, __LINE__);

        hidpReturnCode = HidP_GetValueCaps(HidP_Input, valueCaps, &_numValueCaps, preparsedData);
        if (hidpReturnCode != HIDP_STATUS_SUCCESS)
        {
          fprintf(stderr, "%sHidP_GetValueCaps failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
          utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
          exit(-1);
        }

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

          if (cap.UsagePage == HID_USAGE_PAGE_GENERIC)
          {
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
              // we need to store this information (the link collection ID) so that when we receive WM_INPUT message, we can use this information with the HidP_GetUsageValue function to get the number of touches in that message
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

        PHIDP_BUTTON_CAPS buttonCaps = (PHIDP_BUTTON_CAPS)kankaku_utils_malloc_or_die(sizeof(HIDP_BUTTON_CAPS) * numButtonCaps, __FILE__, __LINE__);

        hidpReturnCode = HidP_GetButtonCaps(HidP_Input, buttonCaps, &_numButtonCaps, preparsedData);
        if (hidpReturnCode != HIDP_STATUS_SUCCESS)
        {
          utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
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

void main_register_input_devices(HWND hwnd)
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
    utils_print_win32_last_error();
    exit(-1);
  }
}

void main_handle_wm_create(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  main_register_input_devices(hwnd);
}

void main_handle_wm_input(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  clock_t ts = clock();
  UINT rawInputSize;
  PRAWINPUT rawInputData = NULL;

  kankaku_touchpad_get_raw_input_data((HRAWINPUT)lParam, &rawInputSize, (LPVOID*)(&rawInputData));

  // Parse the RAWINPUT data.
  if (rawInputData->header.dwType == RIM_TYPEHID)
  {
    // TODO what does `dwCount` represent?
    DWORD count   = rawInputData->data.hid.dwCount;
    BYTE* rawData = rawInputData->data.hid.bRawData;

    if (count != 0)
    {
      UINT deviceNameLength;
      char* deviceName = NULL;
      size_t cbDeviceName;

      kankaku_touchpad_get_raw_input_device_name(rawInputData->header.hDevice, &deviceName, &deviceNameLength, &cbDeviceName);

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
          int compareNameResult = strcmp(deviceName, g_app_state->device_info_list.Entries[touchpadIndex].Name);
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
          printf("[%d]", ts);
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
              utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
              exit(-1);
            }

            ULONG numContacts = usageValue;

            printf(FG_BRIGHT_BLUE);
            printf("numContacts: %d\n", numContacts);
            printf(RESET_COLOR);

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
                kankaku_link_collection_info collectionInfo = g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries[linkColIdx];

                if (collectionInfo.HasX && collectionInfo.HasY && collectionInfo.HasContactID && collectionInfo.HasTipSwitch)
                {
                  hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.LinkColID, 0x30, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

                  if (hidpReturnCode != HIDP_STATUS_SUCCESS)
                  {
                    fprintf(stderr, "%sFailed to read x position!%s\n", FG_RED, RESET_COLOR);
                    utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
                    exit(-1);
                  }

                  ULONG xPos = usageValue;

                  hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.LinkColID, 0x31, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
                  if (hidpReturnCode != HIDP_STATUS_SUCCESS)
                  {
                    printf(FG_RED);
                    printf("Failed to read y position!\n");
                    printf(RESET_COLOR);
                    utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
                    exit(-1);
                  }

                  ULONG yPos = usageValue;

                  hidpReturnCode = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.LinkColID, HID_USAGE_DIGITIZER_CONTACT_ID, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
                  if (hidpReturnCode != HIDP_STATUS_SUCCESS)
                  {
                    printf(FG_RED);
                    printf("Failed to read touch ID!\n");
                    printf(RESET_COLOR);
                    utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
                    exit(-1);
                  }

                  ULONG touchId = usageValue;

                  const ULONG maxNumButtons = HidP_MaxUsageListLength(HidP_Input, HID_USAGE_PAGE_DIGITIZER, preparsedHIDData);

                  ULONG _maxNumButtons = maxNumButtons;

                  USAGE* buttonUsageArray = (USAGE*)kankaku_utils_malloc_or_die(sizeof(USAGE) * maxNumButtons, __FILE__, __LINE__);

                  hidpReturnCode = HidP_GetUsages(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.LinkColID, buttonUsageArray, &_maxNumButtons, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

                  if (hidpReturnCode != HIDP_STATUS_SUCCESS)
                  {
                    printf(FG_RED);
                    printf("HidP_GetUsages failed!\n");
                    printf(RESET_COLOR);
                    utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
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

                  printf(FG_GREEN);
                  printf("LinkColId: %d, touchID: %d, tipSwitch: %d, position: (%d, %d)\n", collectionInfo.LinkColID, touchId, isContactOnSurface, xPos, yPos);
                  printf(RESET_COLOR);
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

LRESULT CALLBACK main_process_window_message(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  clock_t ts = clock();

  switch (uMsg)
  {
    case WM_CREATE:
    {
      main_handle_wm_create(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_INPUT:
    {
      main_handle_wm_input(hwnd, uMsg, wParam, lParam);
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

int CALLBACK main_winmain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  // TODO detect and prompt user to select touchpad device and parse its width and height

  // default window width and height values

  int nWidth  = -1;
  int nHeight = -1;

  HID_DEVICE_INFO* inputDevices = g_app_state->device_info_list.Entries;
  unsigned int numInputDevices  = g_app_state->device_info_list.Size;
  if ((inputDevices != NULL) && (numInputDevices != 0))
  {
    // TODO check for valid touchpad device
    for (unsigned int deviceIdx = 0; deviceIdx < numInputDevices; deviceIdx++)
    {
      HID_DEVICE_INFO inputDevice = g_app_state->device_info_list.Entries[deviceIdx];

      if ((inputDevice.LinkColInfoList.Entries == NULL) || (inputDevice.LinkColInfoList.Size == 0))
      {
        printf(FG_BRIGHT_YELLOW);
        printf("Skipping input device #%d at %s:%d!\n", deviceIdx, __FILE__, __LINE__);
        printf(RESET_COLOR);
        continue;
      }
      else
      {
        for (unsigned int linkColIdx = 0; linkColIdx < inputDevice.LinkColInfoList.Size; linkColIdx++)
        {
          kankaku_link_collection_info linkCollectionInfo = inputDevice.LinkColInfoList.Entries[linkColIdx];
          if (linkCollectionInfo.HasX && linkCollectionInfo.HasY)
          {
            // TODO Should we need to parse every single touch link collections? For now, I think one is sufficient.
            // TODO validate values (e.g. 0 or > screen size)
            if (linkCollectionInfo.PhysicalRect.right > nWidth)
            {
              nWidth = linkCollectionInfo.PhysicalRect.right;
            }

            if (linkCollectionInfo.PhysicalRect.bottom > nHeight)
            {
              nHeight = linkCollectionInfo.PhysicalRect.bottom;
            }

            break;
          }
        }
      }
    }
  }
  else
  {
    printf(FG_RED);
    printf("Failed to parse input devices at %s:%d!\n", __FILE__, __LINE__);
    printf(RESET_COLOR);
    return -1;
  }

  WNDCLASSEX wcex;

  wcex.cbSize        = sizeof(WNDCLASSEX);
  wcex.style         = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc   = main_process_window_message;
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
    printf("RegisterClassEx failed at %s:%d\n", __FILE__, __LINE__);
    utils_print_win32_last_error();
    return -1;
  }

  HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_EX_LAYERED, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, hInstance, NULL);

  if (!hwnd)
  {
    printf("CreateWindow failed at %s:%d\n", __FILE__, __LINE__);
    utils_print_win32_last_error();
    return -1;
  }

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

#define PIPE_OUT_BUFFER_SIZE 16777216  // 16 MB
int kankaku_create_pipe_server(char* pipeName)
{
  int retval        = 0;
  HANDLE pipeHandle = INVALID_HANDLE_VALUE;
  // https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createnamedpipea
  pipeHandle = CreateNamedPipeA(                                //
      pipeName,                                                 // lpName
      PIPE_ACCESS_OUTBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE,     // dwOpenMode
      PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,  // dwPipeMode
      1,                                                        // nMaxInstances
      PIPE_OUT_BUFFER_SIZE,                                     // nOutBufferSize
      0,                                                        // nInBufferSize
      0,                                                        // nDefaultTimeOut
      NULL                                                      // lpSecurityAttributes
  );

  if (pipeHandle == INVALID_HANDLE_VALUE)
  {
    fprintf(stderr, "%sCreateNamedPipeA failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
    utils_print_win32_last_error();
    retval = -1;
  }
  else
  {
    // TODO move to another function
    // https://docs.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-connectnamedpipe
    BOOL cnpRetval = 0;
    printf("waiting for client\n");
    while (1)
    {
      cnpRetval = ConnectNamedPipe(pipeHandle, NULL);  // blocking function call until the client is connected
      if (!cnpRetval)
      {
        if (!(GetLastError() == ERROR_PIPE_LISTENING))
        {
          break;
        }
      }
      else
      {
        break;
      }
    }
    if (!cnpRetval)
    {
      retval = -1;
      fprintf(stderr, "%sConnectNamedPipe failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
      utils_print_win32_last_error();
    }
    else
    {
      // TODO send data to client
      kankaku_device_dimensions sampleDeviceDimensions = {.width = 640, .height = 480};

      uint8_t* serializedData        = NULL;
      size_t serializedDataByteCount = 0;
      kankaku_serialize_device_dimensions(sampleDeviceDimensions, &serializedData, &serializedDataByteCount);

      printf("serializedData: ");
      for (int i = 0; i < serializedDataByteCount; i++)
      {
        printf("%x", serializedData[i]);
      }
      printf("\n");

      DWORD numberOfBytesWritten = 0;

      BOOL wfRetval = WriteFile(    //
          pipeHandle,               // hFile
          serializedData,           // lpBuffer
          serializedDataByteCount,  // nNumberOfBytesToWrite
          &numberOfBytesWritten,    // lpNumberOfBytesWritten
          NULL                      // lpOverlapped
      );

      kankaku_utils_free(serializedData, serializedDataByteCount, __FILE__, __LINE__);

      if (!wfRetval)
      {
        retval = -1;
        fprintf(stderr, "%sWriteFile failed at %s:%d%s\n", FG_BRIGHT_RED, __FILE__, __LINE__, RESET_COLOR);
        utils_print_win32_last_error();
      }

      printf("done\n");

      // TODO check number of bytes written
      FlushFileBuffers(pipeHandle);
      DisconnectNamedPipe(pipeHandle);
      CloseHandle(pipeHandle);
    }
  }

  return retval;
}

int main()
{
  // testing wip function
  kankaku_hid_touchpad_list touchpadList;
  kankaku_touchpad_parse_available_devices(&touchpadList);

  printf("number of touchpad devices: %d\n", touchpadList.size);

  int selectedTouchpadIndex = -1;

  if (touchpadList.size < 1)
  {
    printf("You have no compatible touchpad devices for this program.\n");
    // TODO free resources?
    return 0;
  }
  else if (touchpadList.size == 1)
  {
    selectedTouchpadIndex = 0;
  }
  else
  {
    printf("TODO ask user to select a touchpad device\n");
    return 0;
  }

  kankaku_hid_touchpad hidTouchpad = touchpadList.entries[selectedTouchpadIndex];
  printf("device name: %s\n", hidTouchpad.name.ptr);
  printf("device width: %d\n", hidTouchpad.width);
  printf("device height: %d\n", hidTouchpad.height);

  // char* pipeName = "\\\\.\\pipe\\kankaku";
  // printf("creating named pipe '%s'\n", pipeName);
  // return kankaku_create_pipe_server(pipeName);

  return 0;
  // TODO remove testing code

  // initialize application's states
  g_app_state = (ApplicationState*)kankaku_utils_malloc_or_die(sizeof(ApplicationState), __FILE__, __LINE__);

  g_app_state->device_info_list = (HID_DEVICE_INFO_LIST){.Entries = NULL, .Size = 0};

  main_parse_connected_input_devices();

  return main_winmain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
};
