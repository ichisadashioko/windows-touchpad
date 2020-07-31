<!-- https://docs.microsoft.com/en-us/windows/win32/gdi/device-independent-bitmaps -->

# Device-Independent Bitmaps

A device-independent bitmap (DIB) contains a _color table_. A color table describes how pixel values correspond to _RGB_ color values, which describe colors that are produced by emitting light. Thus, a DIB can achieve the proper color scheme on any device. A DIB contains the following color and dimension information:

- The color format of the device on which the rectangular image was created.
- The resolution of the device on which the rectangular image was created.
- The palette for the device on which the image was created.
- An array of bits that maps red, green, blue ([`RGB`][nf-wingdi-rgb]) triplets to pixels in the rectangular image.
- A data-compression identifier that indicates the data compression scheme (if any) used to reduce the size of the array of bits.

The color and dimension information is stored in a [`BITMAPINFO`][ns-wingdi-bitmapinfo] structure, which consists of a [`BITMAPINFOHEADER`][ns-wingdi-bitmapinfoheader] structure followed by two or more [`RGBQUAD`][ns-wingdi-rgbquad] structures. The `BITMAPINFOHEADER` structure specifies the dimensions of the pixel rectangle, describes the device's color technology, and identifies the compression schemes used to reduce the size of the bitmap. The `RGBQUAD` structures identify the colors that appear in the pixel rectangle.

There are two varieties of DIBs:

- A bottom-up DIB, in which the origin lies at the lower-left corner.
- A top-down DIB, in which the origin lies at the upper-left corner.

If the height of a DIB as indicated by the `Height` member of the bitmap information header structure, is a positive value, it is a bottom-up DIB; if the height is a negative value, it is a top-down DIB. Top-down DIBs cannot be compressed.

The color format is specified in terms of a count of color planes and color bits. The count of color planes is always 1; the count of color bits is 1 for monochrome bitmaps, 4 for VGA bitmaps, and 8, 16, 24, or 32 for bitmaps on other color devices. An application retrieves the number of color bits that a particular display (or printer) uses by calling the [`GetDeviceCaps`][nf-wingdi-getdevicecaps] function, specifying `BITSPIXEL` as the second argument.

The resolution of a display device is specified in pixels-per-meter. An application can retrieve the horizontal resolution for a video display, or printer, by following this three-step process.

1. Call the [`GetDeviceCaps`][nf-wingdi-getdevicecaps] function,  specifying `HORZRES` as the second argument.
2. Call [`GetDeviceCaps`] a second time, specifying `HORZSIZE` as the second argument.
3. Divide the first return value by the second return value.

The application can retrieve the vertical resolution by using the same three-step process with different parameters: `VERTRES` in place of `HORZRES`, and `VERTSIZE` in place of `HORZSIZE`.

The palette is represented by an array of `RGBQUAD` structures that specify the red, green, and blue intensity components for each color in a display device's color palette. Each color index in the palette array maps to a specific pixel in the rectangular region associated with the bitmap. The size of this array, in bits, is equipvalent to the width of the rectangle, in pixels, multiplied by the height of the rectangle, in pixels, multiplied by the count of color bits for the device. An application can retrieve the size of the device's palette by calling the `GetDeviceCaps` function, specifying `NUMCOLORS` as the second argument.

Windows supports the compression of the palette array for 8-bpp and 4-bpp bottom-up DIBs. These arrays can be compressed by using the run-length encoding (RLE) scheme. The RLE scheme uses 2-byte values. the first byte specifying the number of consecutive pixels that use a color index and the second byte specifying the index. For more information bout bitmap compression, see the description of the [`BITMAPINFOHEADER`][ns-wingdi-bitmapinfoheader], [`BITMAPFILEHEADER`][ns-wingdi-bitmapfileheader], [`BITMAPV4HEADER`][ns-wingdi-bitmapv4header], and [`BITMAPV5HEADER`][ns-wingdi-bitmapv5header] structures.

An application can create a DIB from a DDB by initializing the required structures and calling the [`GetDIBits`][nf-wingdi-getdibits] function. To determine whether a device supports this function, call the [`GetDeviceCaps`][nf-wingdi-getdevicecaps] function, specifying `RC_DI_BITMAP` as the `RASTERCAPS` flag.

An application that needs to copy a bitmap can use [`TransparentBlt`][nf-wingdi-transparentblt] to copy all pixels in a source bitmap to a destination bitmap except those pixels that match the transparent color.

An application can use a DIB to set pixels on the display device by calling the [`SetDIBitsToDevice`][nf-wingdi-setdibitstodevice] or the [`StretchDIBits`][nf-wingdi-stretchdibits] function. To determine whether a device supports the `SetDIBitsToDevice` function, call the `GetDeviceCaps` function, specifying `RC_DIBTODEV` as the `RASTERCAPS` flag. Specify `RC_STRETCHDIB` as the `RASTERCAPS` flag to determine if the device supports `StretchDIBits`.

An application that simply needs to display a pre-existing DIB can use the `SetDIBitsToDevice` function. For example, spreadsheet application can open existing charts and display them in a window by using the `SetDIBitsToDevice` function. To repeatedly redraw a bitmap in a window, however, the application should use the [`BtBlt`][nf-wingdi-bitblt] function. For example, a multimedia application that combines animated graphics with sound would benefit from calling the `BitBlt` function because it executes faster than `SetDIBitsToDevice`.

[nf-wingdi-rgb]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-rgb
[ns-wingdi-bitmapinfo]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfo
[ns-wingdi-bitmapinfoheader]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
[ns-wingdi-rgbquad]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbquad
[nf-wingdi-getdevicecaps]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getdevicecaps
[ns-wingdi-bitmapfileheader]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
[ns-wingdi-bitmapv4header]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv4header
[ns-wingdi-bitmapv5header]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv5header
[nf-wingdi-getdibits]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getdibits
[nf-wingdi-transparentblt]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-transparentblt
[nf-wingdi-setdibitstodevice]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-setdibitstodevice
[nf-wingdi-stretchdibits]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-stretchdibits
[nf-wingdi-bitblt]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-bitblt
