
#include <Windows.h>
#include <shlwapi.h>

#include "win_filesystem.h"

#pragma comment(lib, "Shlwapi.lib")

namespace win_filesystem
{
	/* 最大経路長 */
	static constexpr size_t kMaxPathLength = 1024;

	/// @brief 動的割り当てを行わない文字列操作
	template<size_t N>
	class StaticWString
	{
	public:
		const wchar_t* data() const { return m_data; }
		size_t size() const { return m_nWritten; }
		bool empty() const { return m_nWritten == 0; }
		const wchar_t front() const { return m_data[0]; }
		const wchar_t back() const { return m_nWritten == 0 ? m_data[0] : m_data[m_nWritten - 1ULL]; }

		std::wstring_view stringView() const
		{
			return std::wstring_view(m_data, m_nWritten);
		}
		/// @brief 文字列連結
		StaticWString& append(std::wstring_view s)
		{
			if (m_nWritten + s.size() < MaxSize)
			{
				wmemcpy(m_data + m_nWritten, s.data(), s.size());
				m_nWritten += s.size();
				m_data[m_nWritten] = L'\0';
			}

			return *this;
		}
		/// @brief 文字列連結
		StaticWString& append(const wchar_t* s, size_t length)
		{
			if (m_nWritten + length < MaxSize)
			{
				wmemcpy(m_data + m_nWritten, s, length);
				m_nWritten += length;
				m_data[m_nWritten] = L'\0';
			}

			return *this;
		}
		/// @brief 文字連結
		void pushBack(const wchar_t c)
		{
			m_data[m_nWritten] = c;
			++m_nWritten;
			m_data[m_nWritten] = L'\0';
		}
		/// @brief 文字挿入
		void insert(const wchar_t c, size_t nPos = 0)
		{
			if (m_nWritten + 1 > MaxSize)return;

			wmemmove(&m_data[nPos + 1], &m_data[nPos], m_nWritten - nPos);
			m_data[nPos] = c;
			++m_nWritten;
		}
		/// @brief 文字列挿入
		void insert(std::wstring_view s, size_t nPos = 0)
		{
			if (s.size() + nPos > MaxSize)return;

			wmemmove(&m_data[nPos + s.size()], &m_data[nPos], m_nWritten - nPos);
			wmemcpy(&m_data[nPos], s.data(), s.size());
			m_nWritten += s.size();
		}
		/// @brief 文字置換
		void replace(const wchar_t cOld, const wchar_t cNew)
		{
			for (size_t i = 0; i < m_nWritten; ++i)
			{
				wchar_t& cRef = m_data[i];
				if (cRef == cOld)
				{
					cRef = cNew;
				}
			}
		}
		/// @brief 文字列置換
		void replace(std::wstring_view strOld, std::wstring_view strNew)
		{
			if (strOld.empty())return;

			for (size_t nLast = 0; nLast < m_nWritten;)
			{
				std::wstring_view s = stringView();
				size_t nPos = s.find(strOld, nLast);
				if (nPos == std::wstring_view::npos)break;

				ptrdiff_t nDiff = static_cast<ptrdiff_t>(strNew.size() - strOld.size());
				if (m_nWritten + nDiff > MaxSize) break;

				wchar_t* pPos = m_data + nPos;
				wmemmove(pPos + strNew.size(), pPos + strOld.size(), m_nWritten - nPos - strOld.size() + 1);
				wmemcpy(pPos, strNew.data(), strNew.size());

				m_nWritten += nDiff;
				nLast = nPos + strNew.size();
			}
		}
		/// @brief 消去
		void clear()
		{
			wmemset(m_data, L'\0', MaxSize);
			m_nWritten = 0;
		}
	private:
		wchar_t m_data[N]{};
		size_t m_nWritten = 0;
		static constexpr size_t MaxSize = sizeof(m_data) / sizeof(wchar_t) - 1;
	};

	using StaticWStringMaxPath = StaticWString<kMaxPathLength>;

	/*指定階層のファイル・フォルダ名一覧取得*/
	static bool CreateFilaNameList(std::wstring_view folderPath, std::wstring_view fileNamePattern, std::vector<std::wstring>& names)
	{
		StaticWStringMaxPath findDataPath;
		findDataPath.append(folderPath);

		bool toFindDirectory = fileNamePattern.empty();

		if (!toFindDirectory)
		{
			if (fileNamePattern.find(L'*') == std::wstring_view::npos)
			{
				findDataPath.pushBack(L'*');
			}
			findDataPath.append(fileNamePattern);
		}
		else
		{
			findDataPath.pushBack(L'*');
		}

		WIN32_FIND_DATAW win32FindData;

		HANDLE hFind = ::FindFirstFileW(findDataPath.data(), &win32FindData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			if (!toFindDirectory)
			{
				do
				{
					/* ファイル一覧 */
					if (!(win32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						names.emplace_back(win32FindData.cFileName);
					}
				} while (::FindNextFileW(hFind, &win32FindData));
			}
			else
			{
				do
				{
					/* フォルダ一覧 */
					if ((win32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (wcscmp(win32FindData.cFileName, L".") != 0 && wcscmp(win32FindData.cFileName, L"..") != 0)
						{
							names.emplace_back(win32FindData.cFileName);
						}
					}
				} while (::FindNextFileW(hFind, &win32FindData));
			}

			::FindClose(hFind);
		}

		return names.size() > 0;
	}

	static bool MakeDirectory(const std::wstring& directoryPath)
	{
		BOOL iRet = ::CreateDirectoryW(directoryPath.c_str(), nullptr);
		if (iRet == 0)
		{
			if (::GetLastError() == ERROR_ALREADY_EXISTS)
			{
				return true;
			}
		}
		else
		{
			return true;
		}

		return false;
	}
}

bool win_filesystem::CreateFilePathList(std::wstring_view folderPath, std::wstring_view fileSpec, std::vector<std::wstring>& paths, bool toAddParent)
{
	if (folderPath.empty())return false;

	StaticWStringMaxPath parentFolderPath;
	parentFolderPath.append(folderPath);
	if (parentFolderPath.back() != L'\\')
	{
		parentFolderPath.pushBack(L'\\');
	}

	std::vector<std::wstring> fileNames;

	if (fileSpec.empty())
	{
		CreateFilaNameList(parentFolderPath.stringView(), fileSpec, fileNames);
	}
	else
	{
		for (size_t nRead = 0;;)
		{
			size_t nPos = fileSpec.find(';', nRead);
			if (nPos == std::wstring_view::npos)
			{
				std::wstring_view s = fileSpec.substr(nRead);
				CreateFilaNameList(parentFolderPath.stringView(), s, fileNames);
				break;
			}

			std::wstring_view s = fileSpec.substr(nRead, nPos);
			CreateFilaNameList(parentFolderPath.stringView(), s, fileNames);
			nRead = nPos + 1;
		}
	}

	/*名前順に整頓*/
	for (size_t i = 0; i < fileNames.size(); ++i)
	{
		size_t nIndex = i;
		for (size_t j = i; j < fileNames.size(); ++j)
		{
			if (::StrCmpLogicalW(fileNames[nIndex].c_str(), fileNames[j].c_str()) > 0)
			{
				nIndex = j;
			}
		}
		std::swap(fileNames[i], fileNames[nIndex]);
	}

	if (toAddParent)
	{
		for (std::wstring& fileName : fileNames)
		{
			fileName.insert(0, parentFolderPath.data(), parentFolderPath.size());
		}
	}

	paths.reserve(paths.size() + fileNames.size());
	for (std::wstring& fileName : fileNames)
	{
		paths.push_back(std::move(fileName));
	}

	return paths.size() > 0;
}

bool win_filesystem::GetFilePathListAndIndex(std::wstring_view path, std::wstring_view fileSpec, std::vector<std::wstring>& paths, size_t& nIndex)
{
	size_t nPos = path.find_last_of(L"\\/");
	if (nPos == std::wstring_view::npos)return false;

	std::wstring_view parentPath = path.substr(0, nPos);

	win_filesystem::CreateFilePathList(parentPath, fileSpec, paths);

	const auto& iter = std::find(paths.begin(), paths.end(), path);
	if (iter != paths.end())
	{
		nIndex = std::distance(paths.begin(), iter);
	}

	return iter != paths.end();
}

std::wstring_view win_filesystem::GetCurrentProcessPath()
{
	static wchar_t s_basePath[kMaxPathLength]{};
	static size_t s_basePathLength = 0;
	if (s_basePath[0] == L'\0')
	{
		static constexpr size_t basePathSize = sizeof(s_basePath) / sizeof(wchar_t);
		DWORD length = ::GetModuleFileNameW(nullptr, s_basePath, basePathSize);
		wchar_t* pEnd = s_basePath + length;
		for (; pEnd != s_basePath; --pEnd)
		{
			if (*pEnd == L'\\' || *pEnd == L'/')break;
		}

		wchar_t* pFileName = pEnd + 1;
		size_t fileNameLength = s_basePath + length - pFileName;
		wmemset(pFileName, L'\0', fileNameLength);

		s_basePathLength = pFileName - s_basePath;
	}

	return std::wstring_view(s_basePath, s_basePathLength);
}

bool win_filesystem::CreateDirectoryInBuffer(std::wstring_view directoryName, wchar_t* dst, size_t dstSize, size_t& nWritten, std::wstring_view basePath)
{
	if (basePath.empty())
	{
		basePath = GetCurrentProcessPath();
	}

	if (dstSize < basePath.size())return false;

	wmemcpy(dst, basePath.data(), basePath.size());
	nWritten = basePath.size();

	size_t nRead = 0;
	if (dst[nWritten - 1ULL] != L'\\' && dst[nWritten - 1ULL] != L'/')
	{
		dst[nWritten++] = L'\\';
		dst[nWritten] = L'\0';

	}
	if (directoryName[0] == L'\\' || directoryName[0] == L'/')++nRead;

	for (;;)
	{
		size_t nPos = directoryName.find_first_of(L"\\/", nRead);
		if (nPos == std::wstring_view::npos)
		{
			const wchar_t* pRead = directoryName.data() + nRead;
			size_t nLength = directoryName.size() - nRead;
			if (dstSize < nWritten + nLength + 1)return false;

			wmemcpy(dst + nWritten, pRead, nLength);
			nWritten += nLength;
			dst[nWritten++] = L'\\';
			dst[nWritten] = L'\0';

			::CreateDirectoryW(dst, nullptr);

			break;
		}

		const wchar_t* pRead = &directoryName[nRead];
		size_t nLength = nPos - nRead;
		if (dstSize < nWritten + nLength + 1)return false;

		wmemcpy(dst + nWritten, pRead, nLength);
		nWritten += nLength;
		dst[nWritten++] = L'\\';
		dst[nWritten] = L'\0';

		::CreateDirectoryW(dst, nullptr);

		nRead = nPos + 1;
	}

	return true;
}

std::wstring win_filesystem::CreateDirectorio(std::wstring_view directoryName, std::wstring_view basePath)
{
	wchar_t buffer[kMaxPathLength]{};
	constexpr size_t bufferSize = sizeof(buffer) / sizeof(wchar_t) - 1;
	size_t nWritten = 0;
	CreateDirectoryInBuffer(directoryName, buffer, bufferSize, nWritten, basePath);

	return std::wstring(buffer, nWritten);
}

std::string win_filesystem::LoadFileAsString(const wchar_t* filePath)
{
	std::string fileData;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD ulSize = INVALID_FILE_SIZE;
	DWORD ulRead = 0;
	BOOL iRet = FALSE;

	hFile = ::CreateFileW(filePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)goto end;

	ulSize = ::GetFileSize(hFile, nullptr);
	if (ulSize == INVALID_FILE_SIZE)goto end;

	fileData.resize(ulSize);
	iRet = ::ReadFile(hFile, &fileData[0], ulSize, &ulRead, nullptr);
	/* To suppress warning C28193 */
	if (iRet == FALSE)goto end;

end:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hFile);
	}

	return fileData;
}

bool win_filesystem::SaveDataToFile(const wchar_t* filePath, const void* pData, unsigned long dataLength, bool toOverWrite)
{
	if (filePath != nullptr)
	{
		HANDLE hFile = ::CreateFileW(filePath, GENERIC_WRITE, 0, nullptr, toOverWrite ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::SetFilePointer(hFile, NULL, nullptr, FILE_END);
			DWORD nWritten = 0;
			BOOL iRet = ::WriteFile(hFile, pData, dataLength, &nWritten, nullptr);
			::CloseHandle(hFile);

			return iRet == TRUE;
		}
	}
	return false;
}

bool win_filesystem::DoesFileExist(const wchar_t* filePath)
{
	WIN32_FILE_ATTRIBUTE_DATA win32FileAttributeData{};
	BOOL iRet = ::GetFileAttributesExW(filePath, GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &win32FileAttributeData);

	return iRet != 0;
}

bool win_filesystem::RenameFile(const wchar_t* filePathOld, const wchar_t* filePathNew)
{
	return ::MoveFileW(filePathOld, filePathNew) != 0;
}
