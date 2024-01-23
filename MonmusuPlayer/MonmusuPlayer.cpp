// MonmusuPlayer.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "MonmusuPlayer.h"

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "win_text.h"

#include "sfml_spine_player.h"

void GetSpineList(const std::wstring& wstrFolderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths)
{
    std::vector<std::wstring> filePaths;
    win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L".txt", filePaths);
    for (const std::wstring& filePath : filePaths)
    {
        /*---------------------
         * 種類  | 名称格式
         * atlas | xx.atlas.txt
         * skel  | xx.skel.txt
         * json  | xx.txt
         *---------------------*/
        if (filePath.rfind(L".atlas") != std::wstring::npos)
        {
            atlasPaths.push_back(win_text::NarrowUtf8(filePath));
        }
        else
        {
            skelPaths.push_back(win_text::NarrowUtf8(filePath));
        }
    }
}

void GetFolderList(const std::wstring& wstrFolderPath, std::vector<std::wstring>& folders, size_t* nIndex)
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
/* ---------- Example ----------
 * Atlas path: /texture/character/r18_scenes/r18_10166_2
 * To
 * Audio path: /sound/voice/ch_10166/r18/10166b1_10166_r18
 *------------------------------*/
bool GetAudioFolderPath(const std::wstring& wstrAtlasFolderPath, std::wstring& wstrAudioFolderPath)
{
    size_t nPos = wstrAtlasFolderPath.find_last_of(L"\\/");
    if (nPos == std::wstring::npos)return false;

    std::wstring wstrCurrent = wstrAtlasFolderPath.substr(nPos + 1);
    nPos = wstrCurrent.find(L"r18_");
    if (nPos == std::wstring::npos)return false;

    size_t nPos2 = wstrCurrent.substr(nPos + 4).rfind(L"_");
    if (nPos2 == std::wstring::npos)return false;

    std::wstring wstrCharacterId = wstrCurrent.substr(nPos + 4, nPos2);

    nPos = wstrAtlasFolderPath.find(L"texture");
    if (nPos == std::wstring::npos)return false;

    std::wstring wstrVoiceFolder = wstrAtlasFolderPath.substr(0, nPos);
    wstrVoiceFolder += L"sound\\voice";

    std::vector<std::wstring> folders;
    win_filesystem::CreateFilePathList(wstrVoiceFolder.c_str(), nullptr, folders);

    auto IsContained = [&wstrCharacterId](const std::wstring& wstr)
        -> bool
        {
            return wcsstr(wstr.c_str(), wstrCharacterId.c_str()) != nullptr;
        };

    auto iter = std::find_if(folders.begin(), folders.end(), IsContained);
    if (iter == folders.end())return false;
    size_t nIndex = std::distance(folders.begin(), iter);

    std::wstring wstrCharacterVoiceFolder = folders.at(nIndex);
    wstrCharacterVoiceFolder += L"\\r18\\";
    wstrCharacterVoiceFolder += wstrCharacterId;
    if (wstrCharacterId.at(0) == L'3')
    {
        /*30005*/
        switch (wstrCurrent.back())
        {
        case L'1':
            wstrCharacterVoiceFolder += L"c1";
            break;
        case L'2':
            wstrCharacterVoiceFolder += L"d1";
            break;
        case L'3':
            wstrCharacterVoiceFolder += L"e1";
            break;
        }
    }
    else
    {
        wstrCharacterVoiceFolder += wstrCurrent.back() == L'1' ? L"a1" : L"b1";
    }
    wstrCharacterVoiceFolder += L"_";
    wstrCharacterVoiceFolder += wstrCharacterId;
    wstrCharacterVoiceFolder += L"_";
    wstrCharacterVoiceFolder += L"r18";

    wstrAudioFolderPath = wstrCharacterVoiceFolder;

    return true;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    std::wstring wstrPickedFolder = win_dialogue::SelectWorkFolder();
    if (!wstrPickedFolder.empty())
    {
        std::vector<std::wstring> folders;
        size_t nFolderIndex = 0;
        GetFolderList(wstrPickedFolder, folders, &nFolderIndex);
        for (;;)
        {
            std::wstring wstrFolderPath = folders.at(nFolderIndex);

            std::vector<std::string> atlasPaths;
            std::vector<std::string> skelPaths;
            GetSpineList(wstrFolderPath, atlasPaths, skelPaths);
            if (skelPaths.empty())break;

            std::wstring wstrAudioFolderPath;
            GetAudioFolderPath(wstrFolderPath, wstrAudioFolderPath);

            std::vector<std::wstring> audioFilePaths;
            win_filesystem::CreateFilePathList(wstrAudioFolderPath.c_str(), L".m4a", audioFilePaths);

            CSfmlSpinePlayer SfmlPlayer;
            bool bRet = SfmlPlayer.SetSpine(atlasPaths, skelPaths, skelPaths.at(0).rfind(".skel") != std::string::npos);
            if (bRet)
            {
                SfmlPlayer.SetAudios(audioFilePaths);

                int iRet = SfmlPlayer.Display();
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