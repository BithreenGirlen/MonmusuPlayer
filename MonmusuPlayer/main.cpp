
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

#include "sfml_spine_player.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    setlocale(LC_ALL, ".utf8");

    if (!mnms::InitialiseSetting())return 0;

    std::wstring wstrPickedFolder = win_dialogue::SelectWorkFolder(nullptr);
    if (!wstrPickedFolder.empty())
    {
        CSfmlSpinePlayer SfmlPlayer;
        SfmlPlayer.SetFont(mnms::GetFontFilePath(), true, true);

        std::vector<std::wstring> folders;
        size_t nFolderIndex = 0;
        win_filesystem::GetFilePathListAndIndex(wstrPickedFolder, nullptr, folders, &nFolderIndex);
        for (;;)
        {
            std::wstring wstrFolderPath = folders.at(nFolderIndex);

            std::vector<std::string> atlasPaths;
            std::vector<std::string> skelPaths;
            mnms::GetSpineList(wstrFolderPath, atlasPaths, skelPaths);
            if (skelPaths.empty())break;

            std::wstring wstrBookPath;
            mnms::DeriveEpisodeJsonPathFromSpineFolderPath(wstrFolderPath, wstrBookPath);
            std::vector<adv::TextDatum> textData;
            if (!wstrBookPath.empty())
            {
                mnms::ReadScenarioFile(wstrBookPath, textData);
            }

            bool bRet = SfmlPlayer.SetSpineFromFile(atlasPaths, skelPaths, mnms::IsSkelBinary());
            if (!bRet)break;

            SfmlPlayer.SetTexts(textData);
            SfmlPlayer.SetSlotExclusionCallback(&mnms::IsSlotToBeLeftOut);

            int iRet = SfmlPlayer.Display(L"Monmusu Player");
            if (iRet == 1)
            {
                ++nFolderIndex;
                if (nFolderIndex >= folders.size())nFolderIndex = 0;
            }
            else if (iRet == 2)
            {
                --nFolderIndex;
                if (nFolderIndex >= folders.size())nFolderIndex = folders.size() - 1;
            }
            else
            {
                break;
            }
        }
    }

    return 0;
}