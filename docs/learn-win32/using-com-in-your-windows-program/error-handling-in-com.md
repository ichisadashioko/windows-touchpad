<!-- https://docs.microsoft.com/en-us/windows/win32/learnwin32/error-handling-in-com -->
# Error Handling in COM

COM uses HRESULT values to indicate the success or failure of a method or function call. Various SDK headers define various `HRESULT` constants. A common set of system-wide codes is defined in `WinError.h`. The following table shows some of those system-wide return codes.

| Constant | Numeric value | Description |
|---|---|---|
| `E_ACCESSDENIED` | `0x80070005` | Access denied. |
| `E_FAIL` | `0x80004005` | Unspecified error. |
| `E_INVALIDARG` | `0x80070057` | Invalid parameter value. |
| `E_OUTOFMEMORY` | `0x8007000E` | Out of memory. |
| `E_POINTER` | `0x80004003` | `NULL` was passed incorrectly for a pointer value. |
| `E_UNEXPECTED` | `0x8000FFFF` | Unexpected condition. |
| `S_OK` | `0x0` | Success. |
| `S_FALSE` | `0x1` | Success. |

All of the constants with the prefix `E_` are error codes. The constants `S_OK` and `S_FALSE` are both success codes. Probably 99% of COM methods return `S_OK` when they succeed; but do not let this fact mislead you. A method might return other success codes, so always test for errors by using [`SUCCEEDED`][winerror-succeeded] or [`FAILED`][winerror-failed] macro. The following example code shows the wrong way and the right way to test for the success of a function call.

```cpp
// Wrong.
HRESULT hr = SomeFunction();
if (hr != S_OK)
{
    printf("Error!\n"); // Bad. hr might be another success code.
}

// Right.
HRESULT hr = SomeFunction();
if (FAILED(hr))
{
    printf("Error!\n");
}
```

The success code `S_FALSE` deserves mention. Some methods use `S_FALSE` to mean, roughly, a negative condition that is not a failure. It can also indicate a "no-op" - the method succeeded, but had no effect. For example, the [`CoInitializeEx`][coinitializeex] function returns `S_FALSE` if you call it a second time from the same thread. If you need to differentiate between `S_OK` and `S_FALSE` in your code, you should test the value directly, but still use [`FAILED`][winerror-failed] or [`SUCCEEDED`][winerror-succeeded] to handle the remaining cases, as shown in the following example code.

```cpp
if (hr == S_FALSE)
{
    // Handle special case.
}
else if (SUCCEEDED(hr))
{
    // Handle general success case.
}
else
{
    // Handle errors.
    printf("Error!\n");
}
```

Some `HRESULT` values are specific to a particular feature or subsystem of Windows. For example, the Direct2D graphics API defines the error code `D2DERR_UNSUPPORTED_PIXEL_FORMAT`, which means that the program used an unsupported pixel format. MSDN documentation often gives a list of specific error codes that a method might return. However, you should not consider these lists to be definitive. A method can always return an `HRESULT` value that is not listed in the documentation. Again, use the [`SUCCEEDED`][winerror-succeeded] and [`FAILED`][winerror-failed] macros. If you test for a specific error code, include a default case as well.

```cpp
if (hr == D2DERR_UNSUPPORTED_PIXEL_FORMAT)
{
    // Handle the specific case of an unsupported pixel format.
}
else if (FAILED(hr))
{
    // Handle other errors.
}
```

## Patterns for Error Handling

This section looks at some patterns for handling COM errors in a structured way. Each pattern has advantages and disadvantages. To some extent, the choice is a matter of taste. If you work on an existing project, it might already have coding guidelines that proscribe a particular style. Regardless of which pattern you adopt, robust code will obey the following rules.

- For every method or function that returns an `HRESULT`, check the return value before proceeding.
- Release resouces after they are used.
- Do not attempt to access invalid or unintialized resources, such as `NULL` pointers.
- Do not try to use a resouce after you release it.

With these rules in mind, here are four patterns for handling errors.

- Nested ifs
- Cascading ifs
- Jump on Fail
- Throw on Fail

### Nested ifs

After every call that returns an `HRESULT`, use an `if` statement to test for success. Then, put the next method call within the scope of the `if` statement. More `if` statements can be nested as deeply as needed. The previous code examples in this module have all used this pattern, but here it is again:

```cpp
HRESULT ShowDialog()
{
    IFileOpenDialog *pFileOpen;

    HRESULT hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));
    if (SUCCEEDED(hr))
    {
        hr = pFileOpen->Show(NULL);
        if (SUCCEEDED(hr))
        {
            IShellItem *pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                // Use pItem (not shown).
                pItem->Release();
            }
        }
        pFileOpen->Release();
    }
    return hr;
}
```

Advantages

- Variables can be declared with minimal scope. For example, `pItem` is not declared until it is used.
- Within each `if` statement, certain invariants are true: All previous calls have succeeded, and all acquired resources are still valid. In the previous example, when the program reaches the innermost `if` statement, both `pItem` and `pFileOpen` are known to be valid.
- It is clear when to release interface pointers and other resources. You release a resource at the end of `if` statement that immediately follows the call that acquired the resource.

Disadvantages

- Some people find deep nesting hard to read.
- Error handling is mixed in with other branching and looping statements. This can make the the overall program logic harder to follow.

### Cascading ifs

After each method call, use an `if` statement to test for success. If the method succeeds, place the next method call inside the `if` block. But instead of nesting further `if` statements, place each subsequent [`SUCCEEDED`][winerror-succeeded] test after the previous `if` block. If any method fails, all the remaining `SUCCEEDED` tests simply fail until the buttom of the function is reached.

```cpp
HRESULT ShowDialog()
{
    IFileOpenDialog *pFileOpen = NULL;
    IShellItem *pItem = NULL;

    HRESULT hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));

    if (SUCCEEDED(hr))
    {
        hr = pFileOpen->Show(NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = pFileOpen->GetResult(&pItem);
    }
    if (SUCCEEDED(hr))
    {
        // Use pItem (not shown).
    }

    // Clean up.
    SafeRelease(&pItem);
    SafeRelease(&pFileOpen);
    return hr;
}
```

In this pattern, you release resources at the very end of the function. If an error occurs, some pointers might be invalid when the function exits. Calling [`Release`][iunknown-release] on an invalid pointer will crash the proram (or worse), so you must initialize all pointers to `NULL` and check whether they are `NULL` before releasing them. This example uses the `SafeRelease` function; smart pointers are also a good choice.

If you use this pattern, you must be careful with loop constructs, Inside a loop, break from the loop if any call fails.

Advantages

- This pattern creates less nesting than the "nested ifs" pattern.
- Overall control flow is easier to see.
- Resources are released at one point in the code.

Disadvantages

- All variables must be declared and initialized at the top of the function.
- If a call fails, the function make multiple unneeded error checks, instead of exiting the function immediately.
- Because the flow of control continues through the function after a failure, you must be careful throughout the body of the function not to access invalid resources.
- Errors inside a loop require a special case.

### Jump on Fail

After each method call test for failure (not success). On failure, jump to a label near the bottom of the function. After the label, but before exiting the function, release resources.

```cpp
HRESULT ShowDialog()
{
    IFileOpenDialog *pFileOpen = NULL;
    IShellItem *pItem = NULL;

    HRESULT hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pFileOpen->Show(NULL);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pFileOpen->GetResult(&pItem);
    if (FAILED(hr))
    {
        goto done;
    }

    // Use pItem (not shown).

done:
    // Clean up.
    SafeRelease(&pItem);
    SafeRelease(&pFileOpen);
    return hr;
}
```

Advantages

- The overall control flow is easy to see.
- At every point in the code after a [`FAILED`][winerror-failed] check, if you have not jumped to the label, it is guaranteed that all the previous calls have succeeded.
- Resources are released at one place in the code.

Disadvantages

- All variables must be declared and initialized at the top of the function.
- Some programmers do not like to use `goto` in their code. (However, it should be noted that this use of `goto` is highly structured; the code never jumps outside the current function call.)
- `goto` statements skip initializers.

### Throw on Fail

Rather than jump to a label, you can throw an exception when a method fails. This can produce a more idiomatic style of C++ if you are used to writing exception-safe code.

```cpp
#include <comdef.h> // Declares _com_error

inline void throw_if_fail(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw _com_error(hr);
    }
}

void ShowDialog()
{
    try
    {
        CComPtr<IFileOpenDialog> pFileOpen;
        throw_if_fail(CoCreateInstance(__uuidof(FileOpenDialog), NULL), CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));

        throw_if_fail(pFileOpen->Show(NULL));

        CComPtr<IShellItem> pItem;
        throw_if_fail(pFileOpen->GetResult(&pItem));

        // Use pItem (not shown).
    }
    catch (_com_error err)
    {
        // Handle error.
    }
}
```

Notice that this example uses the `CComPtr` class to manage interface pointers. Generally, if your code throws exceptions, you should follow the RAII (Resource Acquisition is Initialization) pattern. That is, every resource should be managed by an object whose destructor guarantees that the resource is correctly released. If an exception is thrown, the destructor is guaranteed to be invoked. Otherwise, your program might leak resources.

Advantages

- Compatible with existing code that uses exception handling.
- Compatible with C++ libraries that throw exceptions, such as the Standard Template Library (STL).

Disadvantages

- Requires C++ objects to manage resources such as memory or file handles.
- Requires a good understanding of how to write exception-safe code.

## Next

[Module 3. Windows Graphics]()

[winerror-succeeded]: https://docs.microsoft.com/en-us/windows/win32/api/winerror/nf-winerror-succeeded
[winerror-failed]: https://docs.microsoft.com/en-us/windows/win32/api/winerror/nf-winerror-failed
[coinitializeex]: https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex
[iunknown-release]: https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release
