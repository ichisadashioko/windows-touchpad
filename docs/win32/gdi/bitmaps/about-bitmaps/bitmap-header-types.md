<!-- https://docs.microsoft.com/en-us/windows/win32/gdi/bitmap-header-types -->

# Bitmap Header Types

The bitmap has four basic header types:

- [`BITMAPCOREHEADER`][ns-wingdi-bitmapcoreheader]
- [`BITMAPINFORHEADER`][ns-wingdi-bitmapinfoheader]
- [`BITMAPV4HEADER`][ns-wingdi-bitmapv4header]
- [`BITMAPV5HEADER`][ns-wingdi-bitmapv5header]

The four types of bitmap headers are differentiated by the `Size` member, which is the first `DWORD` in each of the structures.

The [`BITMAPV5HEADER`][ns-wingdi-bitmapv5header] structure is an extended [`BITMAPV4HEADER`][ns-wingdi-bitmapv4header] structure, which is an extended [`BITMAPINFOHEADER`][ns-wingdi-bitmapinfoheader] structure. However, they `BITMAPINFOHEADER` and [`BITMAPCOREHEADER`][ns-wingdi-bitmapcoreheader] have only the `Size` member in common with other bitmap header structures.

The [`BITMAPCOREHEADER`][ns-wingdi-bitmapcoreheader] and [`BITMAPV4HEADER`][ns-wingdi-bitmapv4header] formats have been superseded by [`BITMAPINFOHEADER`][ns-wingdi-bitmapinfoheader] and [`BITMAPV5HEADER`][ns-wingdi-bitmapv5header] formats, respectively. The `BITMAPCOREHEADER` and `BITMAPV4HEADER` formats are presented for completeness and backward compatibility.

The format for a DIB is the following (for more information, see [`Bitmap Storage`](./bitmap-storage.md)):

- a [`BITMAPFILEHEADER`][ns-wingdi-bitmapfileheader] structure
- either a [`BITMAPCOREHEADER`][ns-wingdi-bitmapcoreheader], a [`BITMAPINFOHEADER`][ns-wingdi-bitmapinfoheader], a [`BITMAPV4HEADER`][ns-wingdi-bitmapv4header], or a [`BITMAPV5HEADER`][ns-wingdi-bitmapv5header] structure.
- an optional color table, which is either a set of [`RGBQUAD`][ns-wingdi-rgbquad] structures or a set of [`RGBTRIPLE`][ns-wingdi-rgbtriple] structures.
- the bitmap data
- optional Profile data

A color table describes how pixel values correspond to RGB color values. RGB is a model for describing colors that are produced by emitting light.

_Profile data_ refers to either the profile file name (linked profile) or the actual profile bits (embedded profile). The file format places the profile data at the end of the file. The profile data is placed just after the color table (if present). However, if the function receives a packed DIB, the profile data comes after the bitmap bits, like in the file format.

Profile data will only exist for [`BITMAPV5HEADER`][ns-wingdi-bitmapv5header] structures where `bV5CSType` is `PROFILE_LINKED` or `PROFILE_EMBEDDED`. For functions that receive packed DIBs, the profile data come after the bitmap data.

A palettized device is any device that uses palettes to assign colors. The classic example of a palettized device is a display running in 8 bit color depth (that is, 256 colors). The display in this mode uses a smal color table to assign colors to bitmap. The colors in a bitmap are assigned to the closest color in the palette that the device is using. The paettized device does not create an optimal palette for displaying the bitmap; it simply uses whatever is in the current palette. Applications are responsible for creating a palette and selecting it into the system. In general, 16-, 24-, and 32-bits-per-pixel (bpp) bitmaps do not contain color tables (a.k.a. optimal palettes for the bitmap); the application is responsible for generating an optimal palette in this case. However, 16-, 24-, and 32-bpp bitmaps can contain such optimal color tables for displaying on palettized devices; in this case the application just needs to create a palette based off the color table present in the bitmap file.

[ns-wingdi-bitmapcoreheader]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapcoreheader
[ns-wingdi-bitmapinfoheader]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
[ns-wingdi-bitmapv4header]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv4header
[ns-wingdi-bitmapv5header]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv5header
[ns-wingdi-bitmapfileheader]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
[ns-wingdi-rgbquad]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbquad
[ns-wingdi-rgbtriple]: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbtriple
