#ifndef WIN_DIALOGUE_H_
#define WIN_DIALOGUE_H_

#include <string>

namespace win_dialogue
{
	std::wstring SelectWorkFolder(void *hParentWnd);
	std::wstring SelectOpenFile(const wchar_t* pwzFileType, const wchar_t* pwzSpec, void* hParentWnd);
	void ShowMessageBox(const char* pzTitle, const char* pzMessage);
}
#endif // WIN_DIALOGUE_H_