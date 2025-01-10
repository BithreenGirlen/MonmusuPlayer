
#include <Windows.h>

#include "mnms.h"

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "win_text.h"

#include "deps/nlohmann/json.hpp"

namespace mnms
{
    std::wstring g_wstrVoiceFolderPath;

    bool SetVoiceFolderPath(const std::wstring& wstrScenarioFiLePath)
    {
        size_t nPos = wstrScenarioFiLePath.rfind(L"advscene\\");
        if (nPos == std::wstring::npos)return false;
        g_wstrVoiceFolderPath = wstrScenarioFiLePath.substr(0, nPos);
        g_wstrVoiceFolderPath += L"advscene\\resources\\advscene\\sound\\voice\\";
        return true;
    }

    std::wstring VoiceRelativeUrlToFilePath(const std::wstring& wstrRelativePath)
    {
        if (g_wstrVoiceFolderPath.empty())return std::wstring();

        std::wstring wstr = wstrRelativePath;
        for (;;)
        {
            size_t nPos = wstr.find(L'/');
            if (nPos == std::wstring::npos)break;
            wstr.replace(nPos, 1, L"\\");
        }

        return g_wstrVoiceFolderPath + wstr + L".m4a";
    }
}
void mnms::GetSpineList(const std::wstring& wstrFolderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths)
{
    std::vector<std::wstring> filePaths;
    win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L".txt", filePaths);
    for (const std::wstring& filePath : filePaths)
    {
        /*---------------------
         * éÌóﬁ  | ñºèÃ
         * atlas | xx.atlas.txt
         * skel  | xx.skel.txt
         * json  | xx.txt
         *---------------------*/
        if (filePath.rfind(L".atlas") != std::wstring::npos)
        {
            atlasPaths.push_back(win_text::NarrowANSI(filePath));
        }
        else
        {
            skelPaths.push_back(win_text::NarrowANSI(filePath));
        }
    }
}
/* ---------- Example ----------
 * Atlas path: /texture/character/r18_scenes/r18_10166_2
 * To
 * Audio path: /sound/voice/ch_10166/r18/10166b1_10166_r18
 *------------------------------*/
bool mnms::DeriveAudioFolderPathFromSpineFolderPath(const std::wstring& wstrAtlasFolderPath, std::wstring& wstrAudioFolderPath)
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
        case L'4':
            wstrCharacterVoiceFolder += L"f1";
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
/* ---------- Example ----------
 * Atlas path: /advscene/resources/advscene/texture/character/r18_scenes/r18_10166_2
 * To
 * Book path: /advscene/scenarioexcel/character/10166
 *------------------------------*/
bool mnms::DeriveEpisodeJsonPathFromSpineFolderPath(const std::wstring& wstrAtlasFolderPath, std::wstring& wstrEpisodeJsonFilePath)
{
    /*âòÇÁÇµÇ¢Ç™ã§í èàóùÇ…ÇπÇ∏é ÇµèëÇ´ÅBó]óTÇ™Ç†ÇÍÇŒêÆóù*/
    size_t nPos = wstrAtlasFolderPath.find_last_of(L"\\/");
    if (nPos == std::wstring::npos)return false;

    std::wstring wstrCurrent = wstrAtlasFolderPath.substr(nPos + 1);
    nPos = wstrCurrent.find(L"r18_");
    if (nPos == std::wstring::npos)return false;

    size_t nPos2 = wstrCurrent.substr(nPos + 4).rfind(L"_");
    if (nPos2 == std::wstring::npos)return false;

    std::wstring wstrCharacterId = wstrCurrent.substr(nPos + 4, nPos2);

    nPos = wstrAtlasFolderPath.find(L"advscene");
    if (nPos == std::wstring::npos)return false;

    std::wstring wstrBookFolder = wstrAtlasFolderPath.substr(0, nPos);
    wstrBookFolder += L"advscene\\scenarioexcel\\character";

    std::vector<std::wstring> folders;
    win_filesystem::CreateFilePathList(wstrBookFolder.c_str(), nullptr, folders);

    auto IsContained = [&wstrCharacterId](const std::wstring& wstr)
        -> bool
        {
            return wcsstr(wstr.c_str(), wstrCharacterId.c_str()) != nullptr;
        };

    auto iter = std::find_if(folders.begin(), folders.end(), IsContained);
    if (iter == folders.end())return false;
    size_t nIndex = std::distance(folders.begin(), iter);

    std::wstring wstrBookJson = folders.at(nIndex);
    wstrBookJson += L"\\";
    wstrBookJson += wstrCharacterId;
    if (wstrCharacterId.at(0) == L'3')
    {
        /*30005*/
        switch (wstrCurrent.back())
        {
        case L'1':
            wstrBookJson += L"c1";
            break;
        case L'2':
            wstrBookJson += L"d1";
            break;
        case L'3':
            wstrBookJson += L"e1";
            break;
        case L'4':
            wstrBookJson += L"f1";
            break;
        }
    }
    else
    {
        wstrBookJson += wstrCurrent.back() == L'1' ? L"a1" : L"b1";
    }
    wstrBookJson += L"_R18.book.json";

    wstrEpisodeJsonFilePath = wstrBookJson;

    return true;
}
/*ãrñ{ì«Ç›éÊÇË*/
bool mnms::ReadScenarioFile(const std::wstring& wstrFilePath, std::vector<adv::TextDatum>& textData)
{
    bool bRet = SetVoiceFolderPath(wstrFilePath);
    if (!bRet)return false;

    std::string strScenarioFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
    if (!strScenarioFile.empty())
    {
        std::string strError;

        nlohmann::json nlJson;
        try
        {
            nlJson = nlohmann::json::parse(strScenarioFile);

            const nlohmann::json& jData = nlJson.at("importGridList").at(0).at("rows");

            for (size_t i = 0; i < jData.size(); ++i)
            {
                const nlohmann::json& jRow = jData.at(i).at("strings");

                if (jRow.size() > 9)
                {
                    adv::TextDatum t;
                    const std::string strMsg = std::string(jRow.at(9));
                    if (strMsg != "Text")
                    {
                        t.wstrText = win_text::WidenUtf8(strMsg);
                        if (jRow.size() > 11)
                        {
                            const std::string strVoicePath = std::string(jRow.at(11));
                            if (!strVoicePath.empty())
                            {
                                t.wstrVoicePath = VoiceRelativeUrlToFilePath(win_text::WidenUtf8(strVoicePath));
                            }
                        }
                    }

                    if (!t.wstrText.empty())
                    {
                        textData.push_back(t);
                    }
                }
            }
        }
        catch (nlohmann::json::exception e)
        {
            strError = e.what();
        }

        if (!strError.empty())
        {
            ::MessageBoxA(nullptr, strError.c_str(), "Parse error", MB_ICONERROR);
            return false;
        }
    }

    return !textData.empty();
}
