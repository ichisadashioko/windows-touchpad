# Window Messages

A GUI application must respond to events from the user and from the operating system.

- __Events from the user__ include all the ways that someone can interact with your program: mouse clicks, key strokes, touch-screen gestures, and so on.
- __Events from the operating system include anything "outside" of the program that can affect how the program behaves. For example, the user might plug in a new hardware device, or Windows might enter a lower-power state (sleep or hibernate).

These events can occur at any time while the program is running, in almost any order. How do you structure a program whose flow of execution cannot be predicted in advance?

To solve this problem, Windows uses a message-passing model. The operating system communicates with your application window by passing messages to it. A message is simply a numeric code that designates a particular event. For example, if the user presses the left mouse button, the window receives a message that has the following message code.

```cpp
#define WM_LBUTTONDOWN 0x0201
```

Some messages have data associated with them. For example, the [`WM_LBUTTONDOWN`][wm-lbuttondown] message includes the x-coordinate and y-coordinate of the mouse cursor.

To pass a message to a window, the operating system calls the window procedure registered for that window. (And now you know what the window procedure is for.)

## The Message Loop

An application will receive thousands of messages while it runs. (Consider that every keystroke and mouse-button click generates a message.) Additionally, an application can have several windows, each with its own window procedure. How does the program receive all these messages and deliver them to the correct window procedure? The application needs a loop to retrieve the messages and dispatch them to the correct windows.

For each thread that creates a window, the operating system creates a queue for window messages. This queue holds messages for all the windows that are created on that thread. The queue itself is hidden from your program. You cannot manipulate the queue directly. However, you can pull a message from the queue by calling the [`GetMessage`][getmessage] function.

```cpp
MSG msg;
GetMessage(&msg, NULL, 0, 0);
```

This function removes the first message from the head of the queue. If the queue is empty, the function blocks until another message is queued. The fact that [`GetMessage`][getmessage] blocks will not make your program unresponsive. If there are no messages, there is nothing for the program to do. If you have to perform background processing, you can create additional threads that continue to run while `GetMessage` waits for another message. (See [Avoiding Bottlenecks in Your Window Procedure](./writing-the-window-procedure.md).)

The first parameter of [`GetMessage`][getmessage] is the address of a [`MSG`][msg] structure with information about the message. This includes the target window and the message code. The other three parameters let you filter which messages you get from the queue. In almost all cases, you will set these parameters to zero.

Although the [`MSG`][msg] structure contains information about the message, you will almost never examine this structure directly. Instead, you will pass it directly to two other functions.

```cpp
TranslateMessage(&msg);
DispatchMessage(&msg);
```

The [`TranslateMessage`][translatemessage] function is related to keyboard input. It translates keystrokes (key down, key up) into characters. You do not really have to know how this function works; just remember to call it before [`DispatchMessage`][dispatchmessage]. The link to the MSDN documentation will give you more information, if you are curious.

The [`DispatchMessage`][dispatchmessage] function tell the operating system to call the window procedure of the window that is the target of the message. In other words, the operating system looks up the window handle in its table of windows, finds the function pointer associated with the window, and invokes the function.

For example, suppose that the user presses the left mouse button. This causes a chain of events:

1. The operating system puts a [`WM_LBUTTONDOWN`][wm-lbuttondown] message on the message queue.
2. Your program calls the [`GetMessage`][getmessage] function.
3. [`GetMessage`][getmessage] pulls the [`WM_LBUTTONDOWN`][wm-lbuttondown] message from the queue and fills in the [`MSG`][msg] structure.
4. Your program calls the [`TranslateMessage`][translatemessage] and [`DispatchMessage`][dispatchmessage] functions.
5. Inside [`DispatchMessage`][dispatchmessage], the operating system call your window procedure.
6. Your window procedure can either respond to the message or ignore it.

When the window procedure returns, it returns back to [`DispatchMessage`][dispatchmessage]. This returns to the message loop for the next message. As long as your program is running, messages will continue to arrive on the queue. Therefore, you must have a loop that continually pulls messages from the queue and dispatches them. You can think of the loop as doing the following:

```cpp
// WARNING: Don't actually write your loop this way.

while (1)
{
    GetMessage(&msg, NULL, 0, 0);
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
```

As written, of course, this loop would never end. That is where the return value for the [`GetMessage`][getmessage] function comes in. Normally, `GetMessage` returns a nonzero value. When you want to exit the application and break out of the message loop, call the [`PostQuitMessage`][postquitmessage].

```cpp
PostQuitMessage(0);
```

The [`PostQuitMessage`][postquitmessage] function puts a [`WM_QUIT`][wm-quit] message on the message queue. `WM_QUIT` is a special message: it causes [`GetMessage`][getmessage] to return zero, signaling the end of the message loop. Here is the revised message loop.

```cpp
// Correct.

MSG msg = { };
while(GetMessage(&msg, NULL, 0, 0))
{
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
```

As long as [`GetMessage`][getmessage] returns a nonzero value, the expression in the `while` loop evaluates to true. After you call [`PostQuitMessage`][postquitmessage], the expression becomes false and the program breaks out of the loop. (One interesting result of this behavior is that your window procedure never receives a [`WM_QUIT`][wm-quit] message. Therefore, you do not have to have a case statement for this message in your window procedure.)

The next obvious question is when to call [`PostQuitMessage`][postquitmessage]. We'll return to this question in the topic [Closing the Window](./closing-the-window.md), but first we have to write our window procedure.

## Posted Message versus Sent Messages

The previous section talked about messages going onto a queue. Sometimes, the operating system will call a window procedure directly, bypassing the queue.

The terminology for this distinction can be confusing:

- _Posting_ a message means the message goes on the message queue, and is dispatched through the message loop ([`GetMessage`][getmessage] and [`DispatchMessage`][dispatchmessage]).
- _Sending_ a message means the message skips the queue, and the operating system calls the window procedure directly.

For now the difference is not very important. The window procedure handles all messages. However, some messages bypass the queue and go directly to your window procedure. However, it can make a difference if your application communicates between windows. You can find a more thorough discussion of this issue in the topic [About Messages and Message Queues](https://docs.microsoft.com/en-us/windows/win32/winmsg/about-messages-and-message-queues).

## Next

[Writing the Window Procedure](./writing-the-window-procedure.md)

[getmessage]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessage
[postquitmessage]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postquitmessage
[wm-quit]: https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-quit
[dispatchmessage]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage
[translatemessage]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage
[wm-lbuttondown]: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttondown
[msg]: https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-msg
