
#include <shobjidl.h>
#include <atlbase.h>

#include "win_dialogue.h"

struct ComInit
{
	HRESULT m_hrComInit;
	ComInit() : m_hrComInit(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)) {}
	~ComInit() { if (SUCCEEDED(m_hrComInit)) ::CoUninitialize(); }
};

/*フォルダ選択ダイアログ*/
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

std::wstring win_dialogue::SelectOpenFile(const wchar_t* pwzFileType, const wchar_t* pwzSpec, void* hParentWnd)
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

void win_dialogue::ShowMessageBox(const char* pzTitle, const char* pzMessage)
{
	::MessageBoxA(nullptr, pzMessage, pzTitle, MB_ICONERROR);
}
