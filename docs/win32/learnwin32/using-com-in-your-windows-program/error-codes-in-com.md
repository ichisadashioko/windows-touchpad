<!-- https://docs.microsoft.com/en-us/windows/win32/learnwin32/error-codes-in-com -->
# Error Codes in COM

To indicate success or failure, COM methods and functions return a value of type `HRESULT`. An `HRESULT` is a 32-bit integer. The high-order bit of the `HRESULT` signals success or failure. Zero (0) indicates success and 1 indicates failure.

This procudes the following numeric ranges:

- Success codes: 0x0-0x7FFFFFFF.
- Error codes: 0x80000000-0xFFFFFFFF.

A small number of COM methods do not return an `HRESULT` value. For example, the [`AddRef`][addref] and [`Release`][release] methods return unsigned long values. But every COM method that returns an error code does so by returning an `HRESULT` value.

To check whether a COM method succeeds, examine the high-order bit of the returned `HRESULT`. The Windows SDK headers provide two macros that make this easier: the [`SUCCEEDED`][succeeded] macro and the [`FAILED`][failed] macro. The `SUCCEEDED` macro returns `TRUE` if an `HRESULT` is a success code and `FALSE` if it is an error code. The following example checks whether [`CoInitializeEx`][coinitializeex] succeeds.

```cpp
HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

if (SUCCEEDED(hr))
{
    // the function succeeded
}
else
{
    // handle the error
}
```

Sometimes it is more convenient to test the inverse condition. The [`FAILED`][failed] macro does the opposite of [`SUCCEEDED`][succeeded]. It returns `TRUE` for an error code and `FALSE` for a success code.

```cpp
HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

if (FAILED(hr))
{
    // handle the error
}
else
{
    // the function succeeded
}
```

Later in this module, we will look at some practical advice on how to structure your code to handle COM errors. (See [Error Handling in COM](./error-handling-in-com.md).)

## Next

[Creating an Object in COM](./creating-an-object-in-com.md)

[addref]: https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-addref
[release]: https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release
[succeeded]: https://docs.microsoft.com/en-us/windows/win32/api/winerror/nf-winerror-succeeded
[failed]: https://docs.microsoft.com/en-us/windows/win32/api/winerror/nf-winerror-failed
[coinitializeex]: https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex
