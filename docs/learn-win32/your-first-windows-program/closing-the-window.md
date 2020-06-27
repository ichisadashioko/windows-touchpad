<!-- https://docs.microsoft.com/en-us/windows/win32/learnwin32/closing-the-window -->
# Closing the Window

When the user closes a window, that action triggers a sequence of window messages.

The user can close an application window by clicking the `Close` button, or by using a keyboard shortcut such as ALT+F4. Any of these actions causes the window to receive a [`WM_CLOSE`][wm-close] message. The `WM_CLOSE` message gives you an opportunity to prompt the user before closing the window. If you really do want to close the window, call the [`DestroyWindow`][destroywindow] function. Otherwise, simply return zero from the `WM_CLOSE` message, and the operating system will ignore the message and not destroy the window.

Here is an example of how a program might handle [`WM_CLOSE`][wm-close].

```cpp
case WM_CLOSE:
    if (MessageBox(hwnd, l"Really quit?", L"My application", MB_OKCANCEL) == IDOK)
    {
        DestroyWindow(hwnd);
    }
    // Else: User canceled. Do nothing.
    return 0;
```

In this example, the [`MessageBox`][messagebox] function shows a modal dialog that contains __OK__ and __Cancel__ buttons. If the user clicks __OK__, the program call [`DestroyWindow`][destroywindow]. Otherwise, if the user clicks __Cancel__, the call to `DestroyWindow` is skipped, and the window remains open. In either case, return zero to indicate that you handled the message.

If you want to close the window without prompting the user, you could simply call [`DestroyWindow`][destroywindow] without the call to [`MessageBox`][messagebox]. However, there is a shortcut in this case. Recall that [`DefWindowProc`][defwindowproc] executes the default action for any window message. In the case of [`WM_CLSOE`][wm-close], `DefWindowProc` automatically calls `DestroyWindow`. That means if you ignore the `WM_CLOSE` message in your `switch` statement, the window is destroyed by default.

When a window is about to be destroyed, it receives a [`WM_DESTROY`][wm-destroy] message. This message is sent after the window is removed from the screen, but before the destruction occurs (in particular, before any child windows are destroyed).

In your main application window, you will typically respond to [`WM_DESTROY`][wm-destroy] by calling [`PostQuitMessage`][postquitmesssage].

```cpp
case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
```

We saw in the [Window Messages](./window-messages.md) section that [`PostQuitMessage`][postquitmessage] puts a [`WM_QUIT`][wm-quit] message on the message queue, causing the message loop to end.

Here is a flow chart showing the typical way to process [`WM_CLOSE`][wm-close] and [`WM_DESTROY`][wm-destroy] messages:

![](https://docs.microsoft.com/en-us/windows/win32/learnwin32/images/wmclose01.png)

## Next

[Managing Application State](./managing-application-state.md)

[wm-close]: https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-close
[destroywindow]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-destroywindow
[messagebox]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox
[defwindowproc]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-defwindowproca
[wm-destroy]: https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-destroy
[postquitmessage]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postquitmessage
[wm-quit]: https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-quit
