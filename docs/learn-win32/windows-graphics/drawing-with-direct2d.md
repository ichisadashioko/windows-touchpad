<!-- https://docs.microsoft.com/en-us/windows/win32/learnwin32/drawing-with-direct2d -->
# Drawing with Direct2D

After you create your graphics resources, you are ready to draw.

## Drawing an Ellipse

The [Circle](./your-first-direct2d-program.md) program performs very simple drawing logic:

1. Fill the background with a solid color.
2. Draw a filled circle.

![](https://docs.microsoft.com/en-us/windows/win32/learnwin32/images/graphics08.png)

Because the render target is a window (as opposed to a bitmap or other offscreen surface), drawing is done in response to `WM_PAINT` messages. The following code shows the window procedure for the Circle program.

```cpp
LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_PAINT:
            OnPaint();
            return 0;

        // Other messages not shown...
    }

    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
```

Here is the code that draws the circle.

```cpp
void MainWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);

        pRenderTarget->BeginDraw();

        pRenderTarget->Clear( D2D1::ColorF(D2D1::ColorF::SkyBlue) );
        pRenderTarget->FillEllipse(ellipse, pBrush);

        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }

        EndPaint(m_hwnd, &ps);
    }
}
```

The `ID2D1RenderTarget` interface is used for all drawing operations. The program's `OnPaint` method does the following:

1. The `ID2D1RenderTarget::BeginDraw` method signals the start of drawing.
2. The `ID2D1RenderTarget::Clear` method fills the entire render target with a solid color. The color is given as a `D2D1_COLOR_F` structure. You can use the `D2D1::ColorF` class to initialize the structure. For more information, see [Using Color in Direct2D](./using-color-in-direct2d.md).
3. The `ID2D1RenderTarget::FillEllipse` method draws a filled ellipse, using the specified brush for the fill. An ellipse is specified by a center point and the x and y radii (radii is the plural of radius - weird word). If the x and y radii are the same, the result is a circle.
4. The `ID2D1RenderTarget::EndDraw` method signals the completion of drawing for this frame. All drawing operations must be placed between calls to `BeginDraw` and `EndDraw`.

The `BeginDraw`, `Clear`, and `FillEllipse` methods all have a `void` return type. If an error occurs during the execution of any of these methods, the error is signaled through the return value of the `EndDraw` method. The `CreateGraphicsResources` method is shown in the topic [Creating Direct2D Resources](./render-targets-devices-and-resources.md). This method creates the render target and the solid-color brush.

The device might buffer the drawing commands and defer executing them until `EndDraw` is called. You can force the device to execute any pending drawing commands by calling `ID2D1RenderTarget::Flush`. Flushing can reduce performance, however.

## Hanlding Device Loss

While your program is running, the graphics device that you are using might become unavailable. For example, the device can be lost if the display resolution changes, or if the user removes the display adapter. If the device is lost, the render target also becomes invalid, along with any device-dependent resources that were associated with the device. Direct2D signals a lost device by returning the error code `D2DERR_RECREATE_TARGET` from the `EndDraw` method. If you receive this error code, you must re-create the render target and all device-dependent resources.

To discard a resource, simply release the interface for that resource.

```cpp
void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}
```

Creating a resource can be an expensive operation, so do not recreate your resources for every `WM_PAINT` message. Create a resource once, and cache the resource pointer until the resource becomes invalid due to device loss, or until you no longer need that resource.

## The Direct2D Render Loop

Regardless of what you draw, your program should perform a loop similar to following.

1. Create device-independent resources.
2. Render the scene.

    a. Check if a valid render target exists. If not create the render target and the device-dependent resources.
    b. Call `ID2D1RenderTarget::BeginDraw`.
    c. Issue drawing commands.
    d. Call `ID2D1RenderTarget::EndDraw`.
    e. If `EndDraw` returns `D2DERR_RECREATE_TARGET`, discard the render target and device-dependent resources.

3. Repeat step 2 whenever you need to update or redraw the scene.

If the render target is a window, step 2 occurs whenever the window receives a `WM_PAINT` message.

The loop shown here handles device loss by dicarding the device-dependent resources and re-creating them at the start of the next loop (step 2a).

## Next

[DPI and Device-Independent Pixels](./dpi-and-device-independent-pixels.md)
