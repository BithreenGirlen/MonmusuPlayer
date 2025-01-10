

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

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
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    std::wstring wstrPickedFolder = win_dialogue::SelectWorkFolder(nullptr);
    if (!wstrPickedFolder.empty())
    {
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
            if (textData.empty())
            {
                /*.jsonなし・読み取り失敗*/
                std::wstring wstrAudioFolderPath;
                mnms::DeriveAudioFolderPathFromSpineFolderPath(wstrFolderPath, wstrAudioFolderPath);

                std::vector<std::wstring> audioFilePaths;
                win_filesystem::CreateFilePathList(wstrAudioFolderPath.c_str(), L".m4a", audioFilePaths);

                for (size_t i = 0; i < audioFilePaths.size(); ++i)
                {
                    textData.push_back(adv::TextDatum{ L"", audioFilePaths.at(i) });
                }
            }

            CSfmlSpinePlayer SfmlPlayer;
            bool bRet = SfmlPlayer.SetSpineFromFile(atlasPaths, skelPaths, skelPaths.at(0).rfind(".skel") != std::string::npos);
            if (bRet)
            {
                if (!textData.empty())
                {
                    SfmlPlayer.SetFont("C:\\Windows\\Fonts\\yumindb.ttf", true, true);
                    SfmlPlayer.SetTexts(textData);
                }

                int iRet = SfmlPlayer.Display(L"Monmusu Player");
                if (iRet == 1)
                {
                    ++nFolderIndex;
                    if (nFolderIndex > folders.size() - 1)nFolderIndex = 0;
                }
                else if (iRet == 2)
                {
                    --nFolderIndex;
                    if (nFolderIndex > folders.size() - 1)nFolderIndex = folders.size() - 1;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    return 0;
}