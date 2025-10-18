
#include <shobjidl.h>
#include <atlbase.h>

#include "win_dialogue.h"

struct ComInit
{
    HRESULT m_hrComInit;
    ComInit() : m_hrComInit(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)) {}
    ~ComInit() { if (SUCCEEDED(m_hrComInit)) ::CoUninitialize(); }
};


std::wstring win_dialogue::SelectWorkFolder(void* hParentWnd)
{
    ComInit sInit;
    CComPtr<IFileOpenDialog> pFolderDlg;
    HRESULT hr = pFolderDlg.CoCreateInstance(CLSID_FileOpenDialog);

    if (SUCCEEDED(hr)) {
        FILEOPENDIALOGOPTIONS opt{};
        pFolderDlg->GetOptions(&opt);
        pFolderDlg->SetOptions(opt | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);

        if (SUCCEEDED(pFolderDlg->Show(static_cast<HWND>(hParentWnd))))
        {
            CComPtr<IShellItem> pSelectedItem;
            pFolderDlg->GetResult(&pSelectedItem);

            wchar_t* pPath;
            pSelectedItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);

            if (pPath != nullptr)
            {
                std::wstring wstrPath = pPath;
                ::CoTaskMemFree(pPath);
                return wstrPath;
            }
        }
    }

    return std::wstring();
}

std::wstring win_dialogue::SelectOpenFile(const wchar_t* pwzFileType, const wchar_t* pwzSpec, const wchar_t* pwzTitle, void* hParentWnd)
{
    ComInit sInit;
    CComPtr<IFileOpenDialog> pFileDialog;
    HRESULT hr = pFileDialog.CoCreateInstance(CLSID_FileOpenDialog);

    if (SUCCEEDED(hr)) {
        COMDLG_FILTERSPEC filter[1]{};
        filter[0].pszName = pwzFileType;
        filter[0].pszSpec = pwzSpec;
        hr = pFileDialog->SetFileTypes(1, filter);
        if (SUCCEEDED(hr))
        {
            FILEOPENDIALOGOPTIONS opt{};
            pFileDialog->GetOptions(&opt);
            pFileDialog->SetOptions(opt | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);
            if (pwzTitle != nullptr)pFileDialog->SetTitle(pwzTitle);

            if (SUCCEEDED(pFileDialog->Show(static_cast<HWND>(hParentWnd))))
            {
                CComPtr<IShellItem> pSelectedItem;
                pFileDialog->GetResult(&pSelectedItem);

                wchar_t* pPath;
                pSelectedItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);

                if (pPath != nullptr)
                {
                    std::wstring wstrPath = pPath;
                    ::CoTaskMemFree(pPath);
                    return wstrPath;
                }
            }
        }
    }

    return std::wstring();
}

std::vector<std::wstring> win_dialogue::SelectOpenFiles(const wchar_t* pwzFileType, const wchar_t* pwzSpec, const wchar_t* pwzTitle, void* hParentWnd)
{
    ComInit sInit;
    CComPtr<IFileOpenDialog> pFileDialog;
    HRESULT hr = pFileDialog.CoCreateInstance(CLSID_FileOpenDialog);

    std::vector<std::wstring> selectedFilePaths;

    if (SUCCEEDED(hr)) {
        COMDLG_FILTERSPEC filter[1]{};
        filter[0].pszName = pwzFileType;
        filter[0].pszSpec = pwzSpec;
        hr = pFileDialog->SetFileTypes(1, filter);
        if (SUCCEEDED(hr))
        {
            FILEOPENDIALOGOPTIONS opt{};
            pFileDialog->GetOptions(&opt);
            pFileDialog->SetOptions(opt | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT);
            if (pwzTitle != nullptr)pFileDialog->SetTitle(pwzTitle);

            if (SUCCEEDED(pFileDialog->Show(static_cast<HWND>(hParentWnd))))
            {
                CComPtr<IShellItemArray> pSelectedItems;
                hr = pFileDialog->GetResults(&pSelectedItems);
                if (SUCCEEDED(hr))
                {
                    DWORD dwCount = 0;
                    pSelectedItems->GetCount(&dwCount);
                    for (unsigned long i = 0; i < dwCount; ++i)
                    {
                        CComPtr<IShellItem> pItem;

                        hr = pSelectedItems->GetItemAt(i, &pItem);
                        if (SUCCEEDED(hr))
                        {
                            wchar_t* pPath = nullptr;
                            pItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);
                            if (pPath != nullptr)
                            {
                                selectedFilePaths.push_back(pPath);
                                ::CoTaskMemFree(pPath);
                            }
                        }
                    }
                }
            }
        }
    }
    return selectedFilePaths;
}

std::wstring win_dialogue::SelectSaveFile(const wchar_t* pwzFileType, const wchar_t* pwzSpec, const wchar_t* pwzDefaultFileName, void* hParentWnd)
{
    ComInit sInit;
    CComPtr<IFileSaveDialog> pFileDialog;
    HRESULT hr = pFileDialog.CoCreateInstance(CLSID_FileSaveDialog);

    if (SUCCEEDED(hr))
    {
        COMDLG_FILTERSPEC filter[1]{};
        filter[0].pszName = pwzFileType;
        filter[0].pszSpec = pwzSpec;
        hr = pFileDialog->SetFileTypes(1, filter);
        if (SUCCEEDED(hr))
        {
            pFileDialog->SetFileName(pwzDefaultFileName);

            FILEOPENDIALOGOPTIONS opt{};
            pFileDialog->GetOptions(&opt);
            pFileDialog->SetOptions(opt | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);

            if (SUCCEEDED(pFileDialog->Show(static_cast<HWND>(hParentWnd))))
            {
                CComPtr<IShellItem> pSelectedItem;
                pFileDialog->GetResult(&pSelectedItem);

                wchar_t* pPath;
                pSelectedItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);

                if (pPath != nullptr)
                {
                    std::wstring wstrPath = pPath;
                    ::CoTaskMemFree(pPath);
                    return wstrPath;
                }
            }
        }
    }

    return std::wstring();
}
