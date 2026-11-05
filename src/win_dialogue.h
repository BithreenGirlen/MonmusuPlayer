#ifndef WIN_DIALOGUE_H_
#define WIN_DIALOGUE_H_

#include <string>
#include <vector>

namespace win_dialogue
{
	std::wstring SelectFolder(const wchar_t* title, void* hOwnerWnd);
	std::wstring SelectOpenFile(const wchar_t* fileType, const wchar_t* fileSpec, const wchar_t* title, void* hOwnerWnd, bool allowAll = false);
	std::vector<std::wstring> SelectOpenFiles(const wchar_t* fileType, const wchar_t* fileSpec, const wchar_t* title, void* hOwnerWnd, bool allowAll = false);
	std::wstring SelectSaveFile(const wchar_t* fileType, const wchar_t* fileSpec, const wchar_t* defaultFileName, void* hOwnerWnd);

	void ShowErrorMessageBox(const char* pzTitle, const char* pzMessage);
}
#endif // WIN_DIALOGUE_H_