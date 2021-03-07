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

static wchar_t KANKAKU_WINDOW_CLASS_NAME[] = L"kankaku";
static kankaku_hid_touchpad gTouchpad;

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
      wchar_t* deviceName = NULL;
      size_t cbDeviceName;

      kankaku_touchpad_get_raw_input_device_name(rawInputData->header.hDevice, &deviceName, &deviceNameLength, &cbDeviceName);

      printf("[%d]", ts);
      NTSTATUS hidpReturnCode;
      ULONG usageValue;

      PHIDP_PREPARSED_DATA preparsedHIDData = gTouchpad.preparsedData;

      hidpReturnCode = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_DIGITIZER, gTouchpad.contactCountLinkCollectionId, HID_USAGE_DIGITIZER_CONTACT_COUNT, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

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

      if (numContacts > gTouchpad.contactLinkCollections.size)
      {
        // TODO how should we deal with this edge case
        printf(FG_RED);
        printf("number of contacts is greater than Link Collection Array size at %s:%d\n", __FILE__, __LINE__);
        printf(RESET_COLOR);
        exit(-1);
      }
      else
      {
        // TODO
        // for (unsigned int linkColIdx = 0; linkColIdx < numContacts; linkColIdx++)
        // {
        //   kankaku_link_collection_info collectionInfo = g_app_state->device_info_list.Entries[foundHidIdx].LinkColInfoList.Entries[linkColIdx];

        //   if (collectionInfo.HasX && collectionInfo.HasY && collectionInfo.HasContactID && collectionInfo.HasTipSwitch)
        //   {
        //     hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.LinkColID, 0x30, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

        //     if (hidpReturnCode != HIDP_STATUS_SUCCESS)
        //     {
        //       fprintf(stderr, "%sFailed to read x position!%s\n", FG_RED, RESET_COLOR);
        //       utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
        //       exit(-1);
        //     }

        //     ULONG xPos = usageValue;

        //     hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.LinkColID, 0x31, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
        //     if (hidpReturnCode != HIDP_STATUS_SUCCESS)
        //     {
        //       printf(FG_RED);
        //       printf("Failed to read y position!\n");
        //       printf(RESET_COLOR);
        //       utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
        //       exit(-1);
        //     }

        //     ULONG yPos = usageValue;

        //     hidpReturnCode = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.LinkColID, HID_USAGE_DIGITIZER_CONTACT_ID, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
        //     if (hidpReturnCode != HIDP_STATUS_SUCCESS)
        //     {
        //       printf(FG_RED);
        //       printf("Failed to read touch ID!\n");
        //       printf(RESET_COLOR);
        //       utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
        //       exit(-1);
        //     }

        //     ULONG touchId = usageValue;

        //     const ULONG maxNumButtons = HidP_MaxUsageListLength(HidP_Input, HID_USAGE_PAGE_DIGITIZER, preparsedHIDData);

        //     ULONG _maxNumButtons = maxNumButtons;

        //     USAGE* buttonUsageArray = (USAGE*)kankaku_utils_malloc_or_die(sizeof(USAGE) * maxNumButtons, __FILE__, __LINE__);

        //     hidpReturnCode = HidP_GetUsages(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.LinkColID, buttonUsageArray, &_maxNumButtons, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

        //     if (hidpReturnCode != HIDP_STATUS_SUCCESS)
        //     {
        //       printf(FG_RED);
        //       printf("HidP_GetUsages failed!\n");
        //       printf(RESET_COLOR);
        //       utils_print_hidp_error(hidpReturnCode, __FILE__, __LINE__);
        //       exit(-1);
        //     }

        //     int isContactOnSurface = 0;

        //     for (ULONG usageIdx = 0; usageIdx < maxNumButtons; usageIdx++)
        //     {
        //       if (buttonUsageArray[usageIdx] == HID_USAGE_DIGITIZER_TIP_SWITCH)
        //       {
        //         isContactOnSurface = 1;
        //         break;
        //       }
        //     }

        //     free(buttonUsageArray);

        //     printf(FG_GREEN);
        //     printf("LinkColId: %d, touchID: %d, tipSwitch: %d, position: (%d, %d)\n", collectionInfo.LinkColID, touchId, isContactOnSurface, xPos, yPos);
        //     printf(RESET_COLOR);
        //   }
        // }
      }

      free(deviceName);
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
  WNDCLASSEXW windowClass;

  windowClass.cbSize        = sizeof(WNDCLASSEXW);
  windowClass.style         = CS_HREDRAW | CS_VREDRAW;
  windowClass.lpfnWndProc   = main_process_window_message;
  windowClass.cbClsExtra    = 0;
  windowClass.cbWndExtra    = 0;
  windowClass.hInstance     = hInstance;
  windowClass.hIcon         = LoadIcon(hInstance, IDI_APPLICATION);
  windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  windowClass.hbrBackground = (HBRUSH)(0);
  windowClass.lpszMenuName  = NULL;
  windowClass.lpszClassName = KANKAKU_WINDOW_CLASS_NAME;
  windowClass.hIconSm       = LoadIcon(hInstance, IDI_APPLICATION);

  if (!RegisterClassExW(&windowClass))
  {
    printf("RegisterClassExW failed at %s:%d\n", __FILE__, __LINE__);
    utils_print_win32_last_error();
    return -1;
  }

  HWND hwnd = CreateWindowW(                //
      KANKAKU_WINDOW_CLASS_NAME,            // lpClassName
      KANKAKU_WINDOW_CLASS_NAME,            // lpWindowName
      WS_OVERLAPPEDWINDOW | WS_EX_LAYERED,  // dwStyle
      CW_USEDEFAULT,                        // x
      CW_USEDEFAULT,                        // y
      0,                                    // nWidth
      0,                                    // nHeight
      NULL,                                 // hWndParent
      NULL,                                 // hMenu
      hInstance,                            // hInstance
      NULL                                  // lpParam
  );

  if (!hwnd)
  {
    printf("CreateWindowW failed at %s:%d\n", __FILE__, __LINE__);
    utils_print_win32_last_error();
    return -1;
  }

  // we don't show the window
  // ShowWindow(hwnd, nCmdShow);
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
  wprintf(L"device name: %s\n", hidTouchpad.name.ptr);
  printf("device width: %d\n", hidTouchpad.width);
  printf("device height: %d\n", hidTouchpad.height);

  wprintf(L"%s\n", hidTouchpad.name.ptr);
  printf("name.length: %d\n", hidTouchpad.name.length);
  printf("name.size: %zu\n", hidTouchpad.name.byteCount);

  printf("\nhexdump\n\n");
  char* tmpPtr = (char*)hidTouchpad.name.ptr;
  for (size_t i = 0; i < hidTouchpad.name.byteCount; i++)
  {
    printf("%02x", tmpPtr[i]);
  }

  printf("\n");

  printf("\nchar dump\n\n");

  for (unsigned int i = 0; i < hidTouchpad.name.length; i++)
  {
    printf("0");
  }
  printf("\n");

  for (unsigned int i = 0; i < hidTouchpad.name.length; i++)
  {
    wprintf(L"%c", hidTouchpad.name.ptr[i]);
  }

  printf("\n");
  gTouchpad = hidTouchpad;

  // char* pipeName = "\\\\.\\pipe\\kankaku";
  // printf("creating named pipe '%s'\n", pipeName);
  // return kankaku_create_pipe_server(pipeName);

  return 0;
  // TODO remove testing code

  return main_winmain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
  // TOD clean up resources
};
