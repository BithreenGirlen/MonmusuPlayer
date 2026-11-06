
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <locale.h>

/*SFML*/
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "winmm.lib")

#ifdef  _DEBUG
#pragma comment(lib, "sfml-system-d.lib")
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#else
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#endif // _DEBUG

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "mnms.h"

#include "sfml_main_window.h"

struct ComInit
{
	HRESULT m_hrComInit;
	ComInit() : m_hrComInit(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)) {}
	~ComInit() { if (SUCCEEDED(m_hrComInit)) ::CoUninitialize(); }
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	::setlocale(LC_ALL, ".utf8");

	ComInit comInit;

	if (!mnms::InitialiseSetting())return 0;
	CSfmlMainWindow sfmlMainWindow(U"Monmusu Player");

	std::wstring selectedFolderPath = win_dialogue::SelectFolder(L"Select advscene/resources/advscene/texture/character/r18_scenes/r18_XXXXX_Y folder", sfmlMainWindow.getWindow()->getNativeHandle());
	if (!selectedFolderPath.empty())
	{
		sfmlMainWindow.setFont(mnms::GetFontFilePath(), true, true);
		sfmlMainWindow.getWindow()->setFramerateLimit(mnms::GetDisplayRefreshRate());

		std::vector<std::wstring> sceneSpineFolderPaths;
		size_t nFolderPathIndex = 0;
		win_filesystem::GetFilePathListAndIndex(selectedFolderPath, {}, sceneSpineFolderPaths, nFolderPathIndex);
		for (;;)
		{
			const std::wstring& sceneSpineFolderPath = sceneSpineFolderPaths[nFolderPathIndex];

			std::vector<std::string> atlasFilePaths;
			std::vector<std::string> skelFilePaths;
			mnms::GetSpineList(sceneSpineFolderPath, atlasFilePaths, skelFilePaths);
			if (skelFilePaths.empty())break;

			std::wstring scenarioFilePath;
			mnms::DeriveScenarioFilePathFromSpineFolderPath(sceneSpineFolderPath, scenarioFilePath);
			std::vector<adv::TextDatum> textData;
			std::vector<std::string> animationNames;
			if (!scenarioFilePath.empty())
			{
				mnms::ReadScenarioFile(scenarioFilePath, textData, animationNames);
			}

			bool bRet = sfmlMainWindow.setSpineFromFile(atlasFilePaths, skelFilePaths, mnms::IsSkelBinary());
			if (!bRet)break;

			sfmlMainWindow.setScenarioData(textData, animationNames);
			sfmlMainWindow.setSlotExclusionCallback(&mnms::IsSlotToBeLeftOut);

			int iRet = sfmlMainWindow.display();
			if (iRet == 1)
			{
				++nFolderPathIndex;
				if (nFolderPathIndex >= sceneSpineFolderPaths.size())nFolderPathIndex = 0;
			}
			else if (iRet == 2)
			{
				--nFolderPathIndex;
				if (nFolderPathIndex >= sceneSpineFolderPaths.size())nFolderPathIndex = sceneSpineFolderPaths.size() - 1;
			}
			else
			{
				break;
			}
		}
	}

	return 0;
}