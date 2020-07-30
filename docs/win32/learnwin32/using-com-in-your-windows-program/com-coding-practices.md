<!-- https://docs.microsoft.com/en-us/windows/win32/learnwin32/com-coding-practices -->
# COM Coding Practices

This topic describes ways to make your COM code more effective and robust.

## The `__uuidof` Operator

When you build your program, you might get linker errors similar to the following:

`unresolved external symbol "struct _GUID const IID_IDrawable"`

This error means that a GUID constant was declared with external linkage (`extern`), and the linker could not find the definition of the constant. The value of a GUID constant is usually exported from a static library file. If you are using Microsoft Visual C++, you can avoid the need to link a static library by using the `__uuidof` operator. This operator is a Microsoft language extension. It returns a GUID value from an expression. The expression can be an interface type name, a class name, or an interface pointer. Using `__uuidof`, you can create the Common Item Dialog object as follows:

```cpp
IFileOpenDialog *pFileOpen;
hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, CLSCTX_ALL, __uuidof(pFileOpen), reinterpret_cast<void**>(&pFileOpen));
```

The compiler extracts the GUID value from the header, so no library export is necessary.

__Note__

> The GUID value is associated with the type name by declaring `__declspec(uuid( ... ))` in the header. For more information, see the documentation for `__declspec` in the Visual C++ documentation.

## The `IID_PPV_ARGS` Macro

We saw that both [`CoCreateInstance`][cocreateinstance] and [`QueryInterface`][iunknown-queryinterface] require coercing the final parameter to a `void**` type. This creates the potential for a type mismatch. Consider the following code fragment:

```cpp
// Wrong!

IFileOpenDialog *pFileOpen;

hr = CoCreateInstance(
    __uuidof(FileOpenDialog),
    NULL,
    CLSCTX_ALL,
    __uuidof(IFileDialogCustomize)), // The IID does not match the pointer type!
    reinterpret_cast<void**>(&pFileOpen) // Coerce to void**.
    );
```

This code asks for the [`IFileDialogCustomize`][ifiledialogcustomize] interface, but passes in an [`IFileOptionDialog`][ifileoptiondialog] pointer. The `reinterpret_cast` expression circumvents the C++ type system, so the compiler will not catch this error. In the best case, if the object does not implement the requested interface, the call simply fails. In the worst case, the function succeeds and you have a mismatched pointer. In other words, the pointer type does not match the actual `vtable` in memory. As you can imagine, nothing good can happen at that point.

__Note__

> A _vtable_ (virtual method table) is a table of function pointers. The `vtable` is how COM binds a method call to its implementation at runtime. Not coincidentally, vtables are how most C++ compilers implement virtual methods.

The [`IID_PPV_ARGS`][iid_ppv_args] macro helps to avoid this class of error. To use this macro, replace the following code:

```cpp
__uuidof(IFileDialogCustomize), reinterpret_cast<void**>(&pFileOpen)
```

with this:

```cpp
IID_PPV_ARGS(&pFileOpen)
```

The macro automatically inserts `__uuidof(IFileOpenDialog)` for the interface identifier, so it is guaranteed to match the pointer type. Here is the modified (and correct) code:

```cpp
// Right
IFileOpenDialog *pFileOpen;
hr = CoCreateInstance(__uuidof())
```

You can use the same macro with [`QueryInterface`][iunknown-queryinterface]:

```cpp
IFileDialogCustomize *pCustom;
hr = pFileOpen->QueryInterface(IID_PPV_ARGS(&pCustom));
```

## The SafeRelease Pattern

Reference counting is one of those things in programming that is basically easy, but is also tedious, which makes it easy to get wrong. Typical errors include:

- Failing to release an interface pointer when you are done using it. This class of bug will cause your program to leak memory and other resources, because objects are not destroyed.
- Calling [`Release`][iunknown-release] with an invalid pointer. For example, this error can happen if the object was never created. This category of bug will probably cause your program to crash.
- Dereferencing an interface pointer after [`Release`][iunknown-release] is called. This bug may cause your program to crash. Worse, it may cause your program to crash at a random later time, making it hard to track down the original error.

One way to avoid these bugs is to call [`Release`][iunknown-release] through a function that safely releases the pointer. The following code shows a function that does this:

```cpp
template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}
```

This function takes a COM interface pointer as a parameter and does the following:

1. Checks whether the pointer is `NULL`.
2. Calls [`Release`][iunknown-release] if the pointer is not `NULL`.
3. Sets the pointer to `NULL`.

Here is an example of how to use `SafeRelease`:

```cpp
void UseSafeRelease()
{
    IFileOpenDialog *pFileOpen = NULL;

    HRESULT hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));
    if (SUCCEEDED(hr))
    {
        // Use the object.
    }
    SafeRelease(&pFileOpen);
}
```

If [`CoCreateInstance`][cocreateinstance] succeeds the call to `SafeRelease` releases the pointer. If `CoCreateInstance` fails, `pFileOpen` remains `NULL`. The `SafeRelease` function checks for this and skips the call to [`Release`][iunknown-release].

It is also safe to call `SafeRelease` more than once on the same pointer, as shown here:

```cpp
// Redundant, not OK.
SafeRelease(&pFileOpen);
SafeRelease(&pFileOpen);
```

## COM Smart Pointers

The `SafeRelease` function is useful, but it requires you to remember two things:

- Intialize every interface pointer to `NULL`.
- Call `SafeRelease` before each pointer goes out of scope.

As a C++ programmer, you are probably thinking that you shouldn't have to remember either of these things. After all, that's why C++ has constructors and destructors. It would be nice to have a class that wraps the underlying interface pointer and automatically initializes and releases the pointer. In other words, we want something like this:

```cpp
// Warning: This example is not complete.

template <class T>
class SmartPointer
{
    T* ptr;

public:
    SmartPointer(T *p) : ptr(p) { }
    ~SmartPointer()
    {
        if (ptr) { ptr->Release(); }
    }
};
```

The class definition shown here is incomplete, and is not usable as shown. At a minimum, you would need to define a copy constructor, an assignment operator, and a way to access the underlying COM pointer. Fortunately, you don't need to do any of this work, because Microsoft Visual Studio already provides a smart pointer class as part of the Active Template Library (ATL).

The ATL smart pointer class is named `CComPtr`. (There is also a `CComQIPtr` class, which is not discussed here.) Here is the [Open Dialog Box](./example-the-open-dialog-box.md) example rewritten to use `CComPtr`.

```cpp
#include <windows.h>
#include <shobjidl.h>
#include <atlbase.h> // Contains the declaration of CComPtr.

int WINAPT wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CComPtr<IFileOpenDialog> pFileOpen;

        // Create the FileOpenDialog object.
        hr = pFileOpen.CoCreateInstance(__uuidof(FileOpenDialog));
        if (SUCCEEDED(hr))
        {
            // Showw the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                CComPtr<IShellItem> pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
                        CoTaskMemFree(pszFilePath);
                    }
                }

                // pItem goes out of scope.
            }

            // pFileOpen goes out of scope.
        }
        CoUninitialize();
    }
    return 0;
}
```

The main difference between this code  and the original example is that this version does not explicitly call [`Release`][iunknown-release]. When the `CComPtr` instances goes out of scope, the destructor calls `Release` on the underlying pointer.

`CComPtr` is a class template. The template argument is the COM interface type. Internally, `CComPtr` holds a pointer of that type. `CComPtr` overrides `operator->()` and `operator&()` so that the class acts like the underlying pointer. For example, the following code is equivalent to calling the `IFileOpenDialog::Show` method directly:

```cpp
hr = pFileOpen->Show(NULL);
```

`CComPtr` also defines a `CComPtr::CoCreateInstance` method, which calls the COM [`CoCreateInstance`][cocreateinstance] function with some default parameter values. The only required parameter is the class identifier, as the next example shows:

```cpp
hr = pFileOpen.CoCreateInstance(__uuidof(FileOpenDialog));
```

The `CComPtr::CoCreateInstance` method is provided purely as a convenience; you can still call the COM [`CoCreateInstance`] function, if you prefer.

## Next

[Error Handling in COM](./error-handling-in-com.md)

[cocreateinstance]: https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
[iunknown-queryinterface]: https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-queryinterface(q)
[ifiledialogcustomize]: https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifiledialogcustomize
[ifileoptiondialog]: https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifileopendialog
[iid_ppv_args]: https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-iid_ppv_args
[iunknown-release]: https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release
