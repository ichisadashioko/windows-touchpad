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

[cocreateinstance]: https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
[iunknown-queryinterface]: https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-queryinterface(q)
[ifiledialogcustomize]: https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifiledialogcustomize
[ifileoptiondialog]: https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifileopendialog
[iid_ppv_args]: https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-iid_ppv_args
