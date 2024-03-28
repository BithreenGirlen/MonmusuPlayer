
#include <Windows.h>
#include <shlwapi.h>

#include "win_filesystem.h"


namespace win_filesystem
{
	/*ファイルのメモリ展開*/
	char* LoadExistingFile(const wchar_t* pwzFilePath, unsigned long* ulSize)
	{
		HANDLE hFile = ::CreateFile(pwzFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwSize = ::GetFileSize(hFile, nullptr);
			if (dwSize != INVALID_FILE_SIZE)
			{
				char* pBuffer = static_cast<char*>(malloc(static_cast<size_t>(dwSize + 1ULL)));
				if (pBuffer != nullptr)
				{
					DWORD dwRead = 0;
					BOOL iRet = ::ReadFile(hFile, pBuffer, dwSize, &dwRead, nullptr);
					if (iRet)
					{
						::CloseHandle(hFile);
						*(pBuffer + dwRead) = '\0';
						*ulSize = dwRead;

						return pBuffer;
					}
					else
					{
						free(pBuffer);
					}
				}
			}
			::CloseHandle(hFile);
		}

		return nullptr;
	}
}

/*指定階層のファイル・フォルダ一覧作成*/
bool win_filesystem::CreateFilePathList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileExtension, std::vector<std::wstring>& paths)
{
	if (pwzFolderPath == nullptr)return false;

	std::wstring wstrParent = pwzFolderPath;
	wstrParent += L"\\";
	std::wstring wstrPath = wstrParent + L'*';
	if (pwzFileExtension != nullptr)
	{
		wstrPath += pwzFileExtension;
	}

	WIN32_FIND_DATA sFindData;
	std::vector<std::wstring> wstrNames;

	HANDLE hFind = ::FindFirstFile(wstrPath.c_str(), &sFindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (pwzFileExtension != nullptr)
		{
			do
			{
				/*ファイル一覧*/
				if (!(sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					wstrNames.push_back(sFindData.cFileName);
				}
			} while (::FindNextFile(hFind, &sFindData));
		}
		else
		{
			do
			{
				/*フォルダ一覧*/
				if ((sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					if (wcscmp(sFindData.cFileName, L".") != 0 && wcscmp(sFindData.cFileName, L"..") != 0)
					{
						wstrNames.push_back(sFindData.cFileName);
					}
				}
			} while (::FindNextFile(hFind, &sFindData));
		}

		::FindClose(hFind);
	}

	/*名前順に整頓*/
	for (size_t i = 0; i < wstrNames.size(); ++i)
	{
		size_t nIndex = i;
		for (size_t j = i; j < wstrNames.size(); ++j)
		{
			if (::StrCmpLogicalW(wstrNames.at(nIndex).c_str(), wstrNames.at(j).c_str()) > 0)
			{
				nIndex = j;
			}
		}
		std::swap(wstrNames.at(i), wstrNames.at(nIndex));
	}

	for (const std::wstring& wstr : wstrNames)
	{
		paths.push_back(wstrParent + wstr);
	}

	return paths.size() > 0;
}
/*指定フォルダと同階層のフォルダ一覧作成・相対位置取得*/
void win_filesystem::GetFolderListAndIndex(const std::wstring& wstrFolderPath, std::vector<std::wstring>& folders, size_t* nIndex)
{
	std::wstring wstrParent;
	std::wstring wstrCurrent;

	size_t nPos = wstrFolderPath.find_last_of(L"\\/");
	if (nPos != std::wstring::npos)
	{
		wstrParent = wstrFolderPath.substr(0, nPos);
		wstrCurrent = wstrFolderPath.substr(nPos + 1);
	}

	win_filesystem::CreateFilePathList(wstrParent.c_str(), nullptr, folders);

	auto iter = std::find(folders.begin(), folders.end(), wstrFolderPath);
	if (iter != folders.end())
	{
		*nIndex = std::distance(folders.begin(), iter);
	}
}
/*文字列としてファイル読み込み*/
std::string win_filesystem::LoadFileAsString(const wchar_t* pwzFilePath)
{
	DWORD ulSize = 0;
	char* pBuffer = LoadExistingFile(pwzFilePath, &ulSize);
	if (pBuffer != nullptr)
	{
		std::string str;
		str.resize(ulSize);
		memcpy(&str[0], pBuffer, ulSize);

		free(pBuffer);
		return str;
	}

	return std::string();
}

std::wstring win_filesystem::GetCurrentProcessPath()
{
	wchar_t pwzPath[MAX_PATH]{};
	::GetModuleFileName(nullptr, pwzPath, MAX_PATH);
	std::wstring::size_type nPos = std::wstring(pwzPath).find_last_of(L"\\/");
	return std::wstring(pwzPath).substr(0, nPos);
}
