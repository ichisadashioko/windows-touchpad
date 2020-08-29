<!-- https://docs.microsoft.com/en-us/windows/win32/learnwin32/mouse-clicks -->

# Responding to Mouse Clicks

If the user clicks a mouse button while the cursor is over the client area of a window, the window receives one of the following messages.

| Message | Meaning |
|---|---|
| `WM_LBUTTONDOWN` | Left button down |
| `WM_LBUTTONUP` | Left button up |
| `WM_MBUTTONDOWN` | Middle button down |
| `WM_MBUTTONUP` | Middle button up |
| `WM_RBUTTONDOWN` | Right button down |
| `WM_RBUTTONUP` | Right button up |
| `WM_XBUTTONDOWN` | `XBUTTON1` or `XBUTTON2` down |
| `WM_XBUTTONUP` | `XBUTTON1` or `XBUTTON2` up |

Recall that the client area is the position of the window that excludes the frame. For more information about client areas, see [What Is a Windows?](../introduction-to-windows-programing-in-c/what-is-a-window.md)

## Mouse Coordinates

In all of these messages, the `lParam` parameter contains the x- and y-coordinates of the mouse pointer. The lowest 16 bits of `lParam` contain the x-coordinate, and the next 16 bits contain the y-coordinate. Use the [`GET_X_LPARAM`][nf-windowsx-get_x_lparam] and [`GET_Y_LPARAM`][nf-windowsx-get_y_lparam] macros to unpack the coordinates from `lParam`.

```c++
int xPos = GET_X_LPARAM(lParam);
int yPos = GET_Y_LPARAM(lParam);
```

These macros are defined in the header file `WindowsX.h`.

On 64-bit Windows, `lParam` is 64-bit value. The upper 32 bits of `lParam` are not used. The MSDN documentation mentions the "low-order word" and "high-order word" of `lParam`. In the 64-bit case, this means the low- and high-order words of the lower 32 bits. The macros extract the right values, so if you use them, you will be safe.

Mouse coordinates are given in pixels, not device-independent pixels (DIPs), and are measured relative to the client are of the window. Coordinates are signed values. Positions above and to the left of the client area have negative coordinates, which is important if you track the mouse position outside the window. We will see how to do that in a later topic, [Capture Mouse Movement Outside the Window](./mouse-movement.md).

## Additional Flags

The `wParam` parameter contains a bitwise __OR__ of flags, indicating the state of the other mouse buttons plus the `SHIFT` and `CTRL` keys.

| Flag | Meaning |
|---|---|
| `MK_CONTROL` | The CTRL key is down. |
| `MK_LBUTTON` | The left mouse button is down. |
| `MK_MBUTTON` | The middle button is down. |
| `MK_RBUTTON` | The right mouse button is down. |
| `MK_SHIFT` | The `SHIFT` key is down. |
| `MK_XBUTTON1` | The `XBUTTON1` button is down. |
| `MK_XBUTTON2` | The `XBUTTON2` button is down. |

The absence of flag means the corresponding button or key was not pressed. For example, to test whether the `CTRL` key is down:

```c++
if (wParam & MK_CONTROL) {}
```

If you need to find the state of other keys besides `CTRL` and `SHIFT`, use the [`GetKeyState`][nf-winuser-getkeystate] function, which is described in [Keyboard Input](./keyboard-input.md).

The `WM_XBUTTONDOWN` and `WM_XBUTTONUP` window messages apply to both `XBUTTON1` and `XBUTTON2`. The `wParam` parameter indicates which button was clicked.

```c++
UINT button = GET_XBUTTON_WPARAM(wParam);
if (button == XBUTTON1)
{
    // XBUTTON1 was clicked.
}
else if (button == XBUTTON2)
{
    // XBUTTON2 was clicked.
}
```

## Double Clicks

A window does not receive double-click notifications by default. To receive double clicks, set the `CS_DBLCLKS` flag in the `WNDCLASS` structure when you register the window class.

```c++
WNDCLASS wc = {};
wc.style = CS_BDLCLKS;

/* Set other structure members. */

RegisterClass(&wc);
```

If you set the `CS_DBLCLKS` flag as shown, the window will receive double-click notifications. A double click is indicated by a window message with 

[nf-windowsx-get_x_lparam]: https://docs.microsoft.com/en-us/windows/win32/api/windowsx/nf-windowsx-get_x_lparam
[nf-windowsx-get_y_lparam]: https://docs.microsoft.com/en-us/windows/win32/api/windowsx/nf-windowsx-get_y_lparam
[nf-winuser-getkeystate]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
