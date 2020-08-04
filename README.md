# Handwriting touchpad

This project is about making use of touchpad for handwriting. It is heavily inspired by the Apple Trackpad on MacBook. As a poor undergraduate student who can't afford a MacBook, I decided to build a software that can turn my touchpad into a device that can be used for handwriting.

> Why would you want to do handwriting on a computer? Can you just type the word?

I am a fan of practicing writing those crazy difficult Japanese Kanji. As a Japanese learner, you will not know how to type those words presented in Kanji. There is some words (e.g. people names) that even if you know the reading but when you type the reading (e.g. with Microsoft IME), it will never give you the kanji that you want.

## Goal

- [x] Retrieve absolute touchpad touches' positions
- [ ] Block mouse movement while using trackpad for handwriting (`BlockInput` API call)
- [x] Visualize touch input position on GUI
- [ ] Integrate with TensorFlow (threading)

## Current progress

- We have been able to get the absolute touchpad input positions and visualize them on a UI window.
- We are having some problem with rendering the UI because the rendering workload is too heavy and the UI is re-rendered for every touch input events.

## CI status

![C/C++ CI](https://github.com/ichisadashioko/windows-touchpad/workflows/C/C++%20CI/badge.svg)

## Terminology

- `digitizer`, `touchpad`, and `trackpad` are refering to the same things in this project.
- (a) `touch` and (a) `contact` are refering to the representation of the data generated when your finger(s) come(s) into contact with the hardware - the touchpad.
- `HID` - [human interface device](https://en.wikipedia.org/wiki/Human_interface_device) - refers to devices used by human to "communicate" with computers (even though they can be used by bot).

## Progress

- We want to get the figure's position when it comes into contact with the touchpad (hereby refered as raw input data).
- Windows will send raw touchpad input data to GUI desktop application. (I am not sure how to get raw HID input from a console application.)
- Most of the APIs are for C/C++ so I decided to use C/C++ to create native Windows Desktop Application.
- We need to register our Desktop App for raw HID input so that Windows will send us the raw HID input data.
- Registering for raw HID input data is done in `WndProc(HWND, UINT, WPARAM, LPARAM)` function.
- `WndProc` is the event handler for Windows Desktop Application.
- We can register for raw HID input data when receiving `WM_CREATE` message (event).
- In Windows Desktop Application programing, the application receives `messages` from Windows and processes them in a callback function - `WndProc` in our case.
- In order to register for raw HID input data, we only need to call [`RegisterRawInputDevices(PCRAWINPUTDEVICE, UINT, UINT)`][registerrawinputdevices] once.
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
- We can call [`GetRawInputData(HRAWINPUT, UINT, LPVOID, PUINT, UINT)`][getrawinputdata] to get either header or raw input data of the device.
- After getting raw input data, it will be in form of a `byte` pointer. In order to parse the raw input data, we need to do some processing with the preparsed data first.
- The preparsed data can be retrieved with the [`GetRawInputDeviceInfo(HANDLE, UINT, LPVOID, PUINT)`][getrawinputdeviceinfoa] call.
- After getting preparsed data, we have some helper functions from `hid.lib` to get information from the data.
- We have to include the `hidusage.h` and `hidpi.h` headers in mentioned order. `hidpi.h` requires some definitions from `hidusage.h` but it doesn't include the `hidusage.h` header. `clang-format` will sort headers and put `hidpi.h` on top of `hidusage.h` which is a bad thing. They also require `Windows.h` header and they also do not include it.
- We also need to add the `hid.lib` to additional dependencies in the Visual Studio project properties file. If you have multiple target platforms (e.g. `x86` and `x64`) or multiple build configurations (e.g. `Debug` and `Release`), you will have to manually add the `hid.lib` to all of them.

- With some methods from `hid.lib`, we can get the following information:

    - the touchpad size (width and height)
    - maximum number (the number - as all the touch data is) of touches per raw input message
    - the link collection (ID) to get the tip switch value in each raw input message (to tell if the finger is currently on the touchpad surface or the finger is lifted from the touchpad surface)
    - the link collection for each touches (contacts) to get/locate the touch's data (position, tip switch, etc.) in the raw input message

- There is no convenient functions (e.g. `GetDeviceWidth()`) to retrieve those information. We have to call `HidP_GetValuesCaps` to get a array of device's `capabilities`. And we have to iterate through them to check their HID `Usage` and `UsagePage` to see if they contain the data (e.g. the width of the device) that we want.
- However, that array doesn't contain all the information that we need. There is another array called `button capabilities` from the `HidP_GetButtonCaps` API call.

[getrawinputdeviceinfoa]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getrawinputdeviceinfoa
[getrawinputdata]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getrawinputdata
[registerrawinputdevices]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerrawinputdevices
