<!-- https://docs.microsoft.com/en-us/windows/win32/learnwin32/memory-allocation-in-com -->
# Memory Allocation in COM

Sometimes a method allocates a memory buffer on the heap and returns the address of the buffer to the caller. COM defines a pair of functions for allocating and freeing memory on the heap.

- The [`CoTaskMemAlloc`][cotaskmemalloc] function allocates a block of memory.
- The [`CoTaskMemFree`][cotaskmemfree] function frees a block of memory that was allocated with [`CoTaskMemAlloc`][cotaskmemalloc].

We saw an example of this pattern in the [Open dialog box example](./example-the-open-dialog-box.md):

```cpp
PWSTR pszFilePath;
hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
if (SUCCEEDED(hr))
{
    // ...
    CoTaskMemFree(pszFilePath);
}
```

The [`GetDisplayName`][getdisplayname] method allocates memory for a string. Internally, the method calls [`CoTaskMemAlloc`][cotaskmemalloc] to allocate the string. When the method returns, `pszFilePath` points to the memory location of the new buffer. The caller is responsible for calling [`CoTaskMemFree`][cotaskmemfree] to free the memory.

Why does COM define its own memory allocation functions? One reason is to provide an abstraction layer over the heap allocator. Otherwise, some methods might call `malloc` while others called `new`. Then your program would need to call `free` in some cases and `delete` in others, and keeping track of it all would quickly become impossible. The COM allocation functions create a uniform approach.

Another consideration is the fact that COM is a _binary_ standard, so it is not tied to a particular programming language. Therefore, COM cannot rely on any language-specific form of memory allocation.

## Next

[COM Coding Practices](./com-coding-practices.md)

[cotaskmemalloc]: https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cotaskmemalloc
[cotaskmemfree]: https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cotaskmemfree
[getdisplayname]: https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitem-getdisplayname
