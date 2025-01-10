
#include <Windows.h>

#include "mnms.h"

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "win_text.h"

#include "deps/nlohmann/json.hpp"

namespace mnms
{
    struct SPlayerSetting
    {
        std::wstring wstrAtlasExtension = L".atlas.txt";
        std::wstring wstrSkelExtension = L".skel.txt";
        std::wstring wstrVoiceExtension = L".m4a";;
        std::wstring wstrSceneTextExtension = L".json";
        std::string strFontFilePath = "C:\\Windows\\Fonts\\yumindb.ttf";
    };

    static SPlayerSetting g_playerSetting;

    static bool ReadSettingFile(SPlayerSetting& playerSetting)
    {
        std::wstring wstrFilePath = win_filesystem::GetCurrentProcessPath() + L"\\setting.txt";
        std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
        if (strFile.empty())return false;

        std::string strError;

        try
        {
            nlohmann::json nlJson = nlohmann::json::parse(strFile);
            std::string str;

            nlohmann::json& jExtension = nlJson.at("extensions");

            str = std::string(jExtension.at("atlas"));
            playerSetting.wstrAtlasExtension = win_text::WidenUtf8(str);

            str = std::string(jExtension.at("skel"));
            playerSetting.wstrSkelExtension = win_text::WidenUtf8(str);

            str = std::string(jExtension.at("voice"));
            playerSetting.wstrVoiceExtension = win_text::WidenUtf8(str);

            str = std::string(jExtension.at("sceneText"));
            playerSetting.wstrSceneTextExtension = win_text::WidenUtf8(str);

            playerSetting.strFontFilePath = nlJson.at("fontPath");

        }
        catch (nlohmann::json::exception e)
        {
            strError = e.what();
            ::MessageBoxA(nullptr, strError.c_str(), "Setting error", MB_ICONERROR);
        }

        return strError.empty();
    }

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

        return g_wstrVoiceFolderPath + wstr + g_playerSetting.wstrVoiceExtension;
    }

    static void EliminateTag(std::wstring& wstr)
    {
        std::wstring wstrResult;
        wstrResult.reserve(wstr.size());
        int iCount = 0;
        for (const auto& c : wstr)
        {
            if (c == L'<')
            {
                ++iCount;
                continue;
            }
            else if (c == L'>')
            {
                --iCount;
                continue;
            }

            if (iCount == 0)
            {
                wstrResult.push_back(c);
            }
        }
        wstr = wstrResult;
    }
}

bool mnms::InitialiseSetting()
{
    SPlayerSetting playerSetting;
    bool bRet = ReadSettingFile(playerSetting);
    if (bRet)
    {
        g_playerSetting = std::move(playerSetting);
    }

    return g_playerSetting.wstrAtlasExtension != g_playerSetting.wstrSkelExtension;
}

const std::string& mnms::GetFontFilePath()
{
    return g_playerSetting.strFontFilePath;
}

const bool mnms::IsSkelBinary()
{
    const wchar_t* wszBinaryCandidates[] =
    {
        L".skel", L".bin"
    };
    for (size_t i = 0; i < sizeof(wszBinaryCandidates) / sizeof(wszBinaryCandidates[0]); ++i)
    {
        if (g_playerSetting.wstrSkelExtension.find(wszBinaryCandidates[i]) != std::wstring::npos)
        {
            return true;
        }
    }
    return false;
}
/*ï`âÊëfçﬁàÍóóéÊìæ*/
void mnms::GetSpineList(const std::wstring& wstrFolderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths)
{
    bool bAtlasLonger = g_playerSetting.wstrAtlasExtension.size() > g_playerSetting.wstrSkelExtension.size();

    std::wstring& wstrLongerExtesion = bAtlasLonger ? g_playerSetting.wstrAtlasExtension : g_playerSetting.wstrSkelExtension;
    std::wstring& wstrShorterExtension = bAtlasLonger ? g_playerSetting.wstrSkelExtension : g_playerSetting.wstrAtlasExtension;
    std::vector<std::string>& strLongerPaths = bAtlasLonger ? atlasPaths : skelPaths;
    std::vector<std::string>& strShorterPaths = bAtlasLonger ? skelPaths : atlasPaths;

    std::vector<std::wstring> wstrFilePaths;
    win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L"*", wstrFilePaths);

    for (const auto& filePath : wstrFilePaths)
    {
        const auto EndsWith = [&filePath](const std::wstring& str)
            -> bool
            {
                if (filePath.size() < str.size()) return false;
                return std::equal(str.rbegin(), str.rend(), filePath.rbegin());
            };

        if (EndsWith(wstrLongerExtesion))
        {
            strLongerPaths.push_back(win_text::NarrowUtf8(filePath));
        }
        else if (EndsWith(wstrShorterExtension))
        {
            strShorterPaths.push_back(win_text::NarrowUtf8(filePath));
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
    wstrBookJson += L"_R18.book";
    wstrBookJson += g_playerSetting.wstrSceneTextExtension;

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

            /*çÄñ⁄ñºÇÕîÚÇŒÇ∑*/
            for (size_t i = 1; i < jData.size(); ++i)
            {
                const nlohmann::json& jRow = jData.at(i).at("strings");

                if (jRow.size() > 9)
                {
                    adv::TextDatum t;

                    const std::string strMsg = std::string(jRow[9]);
                    if (strMsg.empty())continue;

                    t.wstrText.reserve(128);
                    const std::string strName = std::string(jRow[2]);
                    if (!strName.empty())
                    {
                        t.wstrText = win_text::WidenUtf8(strName);
                        t.wstrText += L":\n";
                    }
                    t.wstrText += win_text::WidenUtf8(strMsg);
                    EliminateTag(t.wstrText);

                    if (jRow.size() > 11)
                    {
                        const std::string strVoicePath = std::string(jRow[11]);
                        if (!strVoicePath.empty())
                        {
                            t.wstrVoicePath = VoiceRelativeUrlToFilePath(win_text::WidenUtf8(strVoicePath));
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

    if (textData.empty())
    {
        /*ë‰ñ{Ç»ÇµÅEì«Ç›éÊÇËé∏îs*/
        std::wstring wstrAudioFolderPath;
        mnms::DeriveAudioFolderPathFromSpineFolderPath(wstrFilePath, wstrAudioFolderPath);

        std::vector<std::wstring> audioFilePaths;
        win_filesystem::CreateFilePathList(wstrAudioFolderPath.c_str(), g_playerSetting.wstrVoiceExtension.c_str(), audioFilePaths);

        for (size_t i = 0; i < audioFilePaths.size(); ++i)
        {
            textData.push_back(adv::TextDatum{ L"", audioFilePaths.at(i) });
        }
    }

    return !textData.empty();
}
/*ï`âÊèúäOîªíËä÷êî*/
bool mnms::IsSlotToBeLeftOut(const char* szSlotName, size_t nSlotNameLength)
{
    const char* szExclusionList[] = { "Mask", "White" };
    if (szSlotName != nullptr)
    {
        for (size_t i = 0; i < sizeof(szExclusionList) / sizeof(szExclusionList[0]); ++i)
        {
            if (strstr(szSlotName, szExclusionList[i]) != nullptr)
            {
                return true;
            }
        }
    }
    return false;
}
