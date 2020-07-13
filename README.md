## Goal

- [x] Retrieve absolute touchpad touches' positions
- [ ] Block mouse movement while using trackpad for handwriting (`BlockInput` API call)
- [ ] Visualize touch input position on GUI
- [ ] Integrate with TensorFlow (threading)

## Steps

- We want to get raw touchpad input.
- Windows will send raw touchpad input data to GUI desktop application. (I am not sure how to get raw HID input from a console application.)
- Most of the APIs are for C/C++ so I decided to use C/C++ to create native Windows Desktop Application.
- We need to register our Desktop App for raw HID input so that Windows will send us the raw HID input data.
- Registering for raw HID input data is done in `WndProc(HWND, UINT, WPARAM, LPARAM)` function.
- `WndProc` is the event handler for Windows Desktop Application.
- We can register for raw HID input data when receiving `WM_CREATE` message (event).
- In Windows Desktop Application programing, the application receives `messages` from Windows and processes them in a callback function - `WndProc` in our case.
- In order to register for raw HID input data, we only need to call [`RegisterRawInputDevices(PCRAWINPUTDEVICE, UINT, UINT)`](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerrawinputdevices) once.
- We want Windows Precision Touchpad devices so our `RAWINPUTDEVICE` entry to be passed into `RegisterRawInputDevices` will look like this

```c
RAWINPUTDEVICE rid;

rid.usUsagePage = HID_USAGE_PAGE_DIGITIZER;
rid.usUsage     = HID_USAGE_DIGITIZER_TOUCH_PAD;
rid.dwFlags     = RIDEV_INPUTSINK;
rid.hwndTarget  = hWnd;
```

- After registering for raw HID input data, Windows will send us raw input data for all Windows Precision Touchpad devices.
- If the device is connected after our register process, Windows will still send that device's raw HID input data to our Desktop Application so there is no need to calling `RegisterRawInputDevices` to get raw HID input data of devices that just has been connected.
- However, we may still need to handle multiple devices.
- The raw HID input data will be availabled with the `WM_INPUT` message (event).
- We can call [`GetRawInputData(HRAWINPUT, UINT, LPVOID, PUINT, UINT)`](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getrawinputdata) to get either header or raw input data of the device.
- After getting raw input data, it will be in form of a `byte` pointer. In order to parse the raw input data, we need to do some processing with the preparsed data first.
- The preparsed data can be retrieved with the [`GetRawInputDeviceInfo(HANDLE, UINT, LPVOID, PUINT)`](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getrawinputdeviceinfoa) call.
