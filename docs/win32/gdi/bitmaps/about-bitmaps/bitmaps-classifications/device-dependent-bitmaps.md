<!-- https://docs.microsoft.com/en-us/windows/win32/gdi/device-dependent-bitmaps -->

# Device-Dependent Bitmaps

Device-dependent bitmaps (DDBs) are discribed by using a single structure, the [`BITMAP`][ns-wingdi-bitmap] structure. The members of this structure specify the width and height of a rectangular region, in pixels; the width of the array that maps entries from the device palette to pixels; and the device's color format of a device by calling the `GetDeviceCaps` function and specifying the appropriate constants. Note that a DDB does not contain color values; instead, the colors are in a device-dependent format. For more information, see [Color in Bitmaps](../../../colors/about-colors/color-basics/color-in-bitmaps.md). Because each device can have its own set of colors, a DDB created for one device may not display well on a different device.

To use a DDB in a device context, it must have the color organization of that device context. Thus, a DDB is often called a _compatible bitmap_ and it usually has better GDI performance than a DIB. For example, to create a bitmap for video memory, it is best to use a compatible bitmap with the same color format as the primary display. Once in video memory, rendering to the bitmap and displaying it to the screen are significantly faster than from a system memory surface or directly form a DIB.

In addition to enabling better GDI performance, compatible bitmaps are used to capture images (see [Capturing an Image](../../using-bitmaps/capturing-an-image.md)) and to create bitmaps at runtime for menus see "Creating the Bitmap" in (see [Using Menus][using-menus]).

To transfer a bitmap between devices with different color organization, use [`GetDIBits`][nf-wingdi-getdibits] to convert the compatible bitmap to a DIB and call [`SetDIBits`][nf-wingdi-setdibits] or [`StretchDIBits`][nf-wingdi-stretchdibits] to display the DIB to the second device.

There are two types of DDBs: discardable and nondiscardable. A discardable DDB is a bitmap that the system discards if the bitmap is not selected into a DC and if system memory is low. The [`CreateDiscardableBitmap`][nf-wingdi-creatediscardablebitmap] function creates discardable bitmaps. The [`CreateBitmap`][nf-wingdi-createbitmap], [`CreateCompatibleBitmap`][nf-wingdi-createcompatiblebitmap], and [`CreateBitmapIndirect`][nf-wingdi-createbitmapindirect] functions create nondiscardable bitmaps.

An application can create a DDB from a DIB by initializing the required structures and calling the [`CreateDIBitmap`][nf-wingdi-createdibitmap] function. Specifying `CBM_INIT` in the call to `CreateDIBitmap` is equivalent to calling the [`CreateCompatibleBitmap`][nf-wingdi-createcompatiblebitmap] function to create a DDB in the format of the device and then calling the [`SetDIBits`][nf-wingdi-setdibits] function to translate the DIB bits to the DDB. To determine whether a device supports the `SetDIBits` function, call the [`GetDeviceCaps`][nf-wingdi-getdevicecaps] function, specifying `RC_DI_BITMAP` as the `RASTERCAPS` flag.

[ns-wingdi-bitmap]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmap
[using-menus]: https://docs.microsoft.com/en-us/windows/win32/menurc/using-menus
[nf-wingdi-getdibits]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getdibits
[nf-wingdi-setdibits]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-setdibits
[nf-wingdi-stretchdibits]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-stretchdibits
[nf-wingdi-creatediscardablebitmap]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-creatediscardablebitmap
[nf-wingdi-createbitmap]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createbitmap
[nf-wingdi-createcompatiblebitmap]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createcompatiblebitmap
[nf-wingdi-createbitmapindirect]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createbitmapindirect
[nf-wingdi-createdibitmap]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createdibitmap
[nf-wingdi-getdevicecaps]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getdevicecaps
