#include <shobjidl.h>
#include <windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr)) {
        IFileOpenDialog* pFileOpen;

        // create the FileOpenDialog object
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            // show the Open dialog box
            hr = pFileOpen->Show(NULL);

            // get the file name from the dialog box
            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user
                    if (SUCCEEDED(hr)) {
                        MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
                        CoTaskMemFree(pszFilePath);
                    }

                    pItem->Release();
                }
            }

            pFileOpen->Release();
        }

        CoUninitialize();
    }

    return 0;
}

int main()
{
    return wWinMain(GetModuleHandleW(NULL), NULL, GetCommandLineW(), SW_SHOWNORMAL);
}