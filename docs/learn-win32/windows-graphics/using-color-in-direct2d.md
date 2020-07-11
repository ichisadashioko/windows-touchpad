<!-- https://docs.microsoft.com/en-us/windows/win32/learnwin32/using-color-in-direct2d -->
# Using Color in Direct2D

Direct2D uses the RGB color model, in which colors are formed by combining different values of red, green, and blue. A fourth component, alpha, measures the transparency of a pixel. In Direct2D, each of these components is a floating-point value with a range of `[0.0 1.0]`. For the three color components, the value measures the intensity of the color. For the alpha component, `0.0` means completely transparent, and `1.0` means completely opaque. The following table shows the colors that result from various combinations of 100% intensity.

| Red | Green | Blue | Color |
|---|---|---|---|
| 0 | 0 | 0 | Black |
| 1 | 0 | 0 | Red |
| 0 | 1 | 0 | Green |
| 0 | 0 | 1 | Blue |
| 0 | 1 | 1 | Cyan |
| 1 | 0 | 1 | Magenta |
| 1 | 1 | 0 | Yellow |
| 1 | 1 | 1 | White |

![](https://docs.microsoft.com/en-us/windows/win32/learnwin32/images/graphics13.png)

Color values between 0 and 1 result in different shades of these pure colors. Direct2D uses the `D2D1_COLOR_F` structure to represent colors. For example, the following code specifies magenta.

```c++
// Initialize a magenta color.

D2D1_COLOR_F clr;
clr.r = 1;
clr.g = 0;
clr.b = 1;
clr.a = 1; // Opaque.
```

You can also specify a color using the `D2D1::ColorF` class, which derives from the `D2D1_COLOR_F` structure.

```c++
// Equivalent to the previous example.

D2D1::ColorF clr(1, 0, 1, 1);
```

## Alpha Blending

Alpha blending creates translucent areas by blending the foreground color with the background color, using the following formula.

```
color = af * Cf + (1 - af) * Cb
```

where `Cb` is the background color, `Cf` is the foreground color, `af` is the alpha value of the foreground color. This formula is applied pairwise to each color component. For example, suppose the foreground coor is `(R = 1.0, G = 0.4, B = 0.0)`, with `alpha = 0.6`, and the background color is `(R = 0.0, G = 0.5, B = 1.0)`. The resulting alpha-blended color is:

```
R = (1.0 * 0.6 + 0 * 0.4) = 0.6
G = (0.4 * 0.6 + 0.5 * 0.4) = 0.44
B = (0 * 0.6 + 1.0 * 0.4) = 0.4
```

The following image shows the result of this blending operation.

![](https://docs.microsoft.com/en-us/windows/win32/learnwin32/images/graphics15.png)

## Pixel Formats

The `D2D1_COLOR_F` structure does not describe how a pixel is represented in memory. In most cases, that doesn't matter. Direct2D handles all of the internal details of translating color information into pixels. But you might need to know the pixel format if you are working directly with a bitmap in memory. or if you combine Direct2D with Direct3D or GDI.

The `DXGI_FORMAT` enumeration defines a list of pixel formats. The list is fairly long, but only a few of them are relevant to Direct2D. (The others are used by Direct3D.)

| Pixel format | Description |
|---|---|
| `DXGI_FORMAT_B8G8R8A8_UNORM` | This is the most common pixel format. All pixel components (red, green, blue, and alpha) are 8-bit unsigned integers. The components are arranged in `BGRA` order in memory. (See illustration that follows.) |
| `DXGI_FORMAT_R8G8B8A8_UNORM` | Pixel components are 8-bit unsigned integers, in `RGBA` order. In other words, the red and blue components are swapped, relative to `DXGI_FORMAT_B8G8R8A8_UNORM`. This format is supported only for hardware devices. |
| `DXGI_FORMAT_A8_UNORM` | This format contains an 8-bit alpha component, with no RGB components. It is useful for creating opacity masks. To read more about using opacity masks in Direct2D, see [Compatible A8 Render Targets Overview](https://docs.microsoft.com/en-us/windows/win32/direct2d/compatible-a8-rendertargets). |

The following illustration shows BGRA pixel layout.

![](https://docs.microsoft.com/en-us/windows/win32/learnwin32/images/graphics14.png)

To get the pixel format of a render target, call `ID2D1RenderTarget::GetPixelFormat`. The pixel format might not match the display resolution. For example, the display might be set to 16-bit color, even though the render target uses 32-bit color.

### Alpha Mode

A render target also has an alpha mode, which defines how the alpha values are treated.

| Alpha mode | Description |
|---|---|
| `D2D1_ALPHA_MODE_IGNORE` | No alpha blending is performed. Alpha values are ignored. |
| `D2D1_ALPHA_MODE_STRAIGHT` | Straight alpha. The color components of the pixel represent the color intensity prior to alpha blending. |
| `D2D1_ALPHA_MODE_PREMULTIPLIED` | Premultiplied alpha. The color components of the pixel represent the color intensity multiplied by the alpha value. This format is more efficient to render than straight alpha, because the term (`af` and `Cf`) from the alpha-blending formula is pre-computed. However, this format is not appropriate for storing in an image file.

Here is example of the different between straight alpha and premultiplied alpha. Suppose the desired color is pure red (100% intensity) with 50% alpha. As a Direct2D type, this color would be represented as `(1, 0, 0, 0.5)`. Using straight alpha, and assuming 8-bit color components, the red component of the pixel is `0xFF`. Using premultiplied alpha, the red component is scaled by 50% to equal `0x80`.

The `D2D1_COLOR_F` data type always represents colors using straight alpha. Direct2D converts pixels to premultiplied alpha format if needed.

If you know that your program will not perform any alpha blending, create the render target with the `D2D1_ALPHA_MODE_IGNORE` alpha mode. This mode can improve performance, because Direct2D can skip the alpha calculations. For more information, see [Improving the Performance of Direct2D Applications](https://docs.microsoft.com/en-us/windows/win32/direct2d/improving-direct2d-performance).

## Next

[Applying Transforms in Direct2D]()
