
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

			const nlohmann::json& jExtension = nlJson.at("extensions");

			str = jExtension.at("atlas");
			playerSetting.wstrAtlasExtension = win_text::WidenUtf8(str);

			str = jExtension.at("skel");
			playerSetting.wstrSkelExtension = win_text::WidenUtf8(str);

			str = jExtension.at("voice");
			playerSetting.wstrVoiceExtension = win_text::WidenUtf8(str);

			str = jExtension.at("sceneText");
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

	struct CommandIndex abstract final
	{
		enum
		{
			Unspecified,
			Command,
			Arg1,
			Arg2,
			Arg3,
			Arg4,
			Arg5,
			Arg6,
			WaitType,
			Text,
			PageCtrl,
			Voice,
			WIndowType
		};
	};

	std::wstring g_wstrVoiceFolderPath;

	static bool SetVoiceFolderPath(const std::wstring& wstrScenarioFiLePath)
	{
		size_t nPos = wstrScenarioFiLePath.rfind(L"advscene\\");
		if (nPos == std::wstring::npos)return false;
		g_wstrVoiceFolderPath = wstrScenarioFiLePath.substr(0, nPos);
		g_wstrVoiceFolderPath += L"advscene\\resources\\advscene\\sound\\voice\\";
		return true;
	}

	static std::wstring VoiceRelativeUrlToFilePath(const std::wstring& wstrRelativePath)
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

	/* 人物ID抽出 */
	static std::wstring ExtractCharacterIdFromSpineFolderPath(const std::wstring& wstrSpineFolderPath)
	{
		size_t nPos = wstrSpineFolderPath.find_last_of(L"\\/");
		if (nPos == std::wstring::npos)return {};

		std::wstring wstrCurrent = wstrSpineFolderPath.substr(nPos + 1);
		nPos = wstrCurrent.find(L"r18_");
		if (nPos == std::wstring::npos)return {};
		/* length of "r18_" */
		size_t nPos2 = wstrCurrent.substr(nPos + 4).rfind(L"_");
		if (nPos2 == std::wstring::npos)return {};

		return wstrCurrent.substr(nPos + 4, nPos2);
	}

	/* ID対応先探索 */
	static std::wstring FindPathContainingId(const std::wstring& targetFolder, const std::wstring& wstrId, const wchar_t* pwzFileExtension)
	{
		std::vector<std::wstring> paths;
		win_filesystem::CreateFilePathList(targetFolder.c_str(), pwzFileExtension, paths);
		const auto IsContained = [&wstrId](const std::wstring& wstr)
			-> bool
			{
				return wcsstr(wstr.c_str(), wstrId.c_str()) != nullptr;
			};

		const auto iter = std::find_if(paths.begin(), paths.end(), IsContained);
		if (iter == paths.cend())return std::wstring();

		size_t nIndex = std::distance(paths.begin(), iter);
		return paths[nIndex];
	}

	static std::wstring ToBookId(const std::wstring& wstrCharacterId, const wchar_t sceneNumber)
	{
		if (!wstrCharacterId.empty() && wstrCharacterId[0] == L'3')
		{
			/*30005*/
			wchar_t id[3] = L"c1";
			id[0] += sceneNumber - '1';
			return id;
		}
		else
		{
			return sceneNumber == L'1' ? L"a1" : L"b1";
		}
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
		wstr = std::move(wstrResult);
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
	constexpr const wchar_t* const wszBinaryCandidates[] =
	{
		L".skel", L".bin"
	};
	for (const auto& wszBinaryCandidate : wszBinaryCandidates)
	{
		if (g_playerSetting.wstrSkelExtension.find(wszBinaryCandidate) != std::wstring::npos)
		{
			return true;
		}
	}

	return false;
}
/*描画素材一覧取得*/
void mnms::GetSpineList(const std::wstring& wstrFolderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths)
{
	bool bAtlasLonger = g_playerSetting.wstrAtlasExtension.size() > g_playerSetting.wstrSkelExtension.size();

	const std::wstring& wstrLongerExtesion = bAtlasLonger ? g_playerSetting.wstrAtlasExtension : g_playerSetting.wstrSkelExtension;
	const std::wstring& wstrShorterExtension = bAtlasLonger ? g_playerSetting.wstrSkelExtension : g_playerSetting.wstrAtlasExtension;
	std::vector<std::string>& strLongerPaths = bAtlasLonger ? atlasPaths : skelPaths;
	std::vector<std::string>& strShorterPaths = bAtlasLonger ? skelPaths : atlasPaths;

	std::vector<std::wstring> wstrFilePaths;
	win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L"*", wstrFilePaths);

	for (const auto& filePath : wstrFilePaths)
	{
		if (filePath.ends_with(wstrLongerExtesion))
		{
			strLongerPaths.emplace_back(win_text::NarrowUtf8(filePath));
		}
		else if (filePath.ends_with(wstrShorterExtension))
		{
			strShorterPaths.emplace_back(win_text::NarrowUtf8(filePath));
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
	std::wstring wstrCharacterId = ExtractCharacterIdFromSpineFolderPath(wstrAtlasFolderPath);
	if (wstrCharacterId.empty())return false;
	const wchar_t sceneNumber = wstrAtlasFolderPath.back();

	size_t nPos = wstrAtlasFolderPath.find(L"texture");
	if (nPos == std::wstring::npos)return false;

	std::wstring wstrVoiceFolder = wstrAtlasFolderPath.substr(0, nPos);
	wstrVoiceFolder += L"sound\\voice";

	std::wstring wstrCharacterVoiceFolder = FindPathContainingId(wstrVoiceFolder, wstrCharacterId, nullptr);
	if (wstrCharacterVoiceFolder.empty())return false;

	wstrCharacterVoiceFolder += L"\\r18\\";
	wstrCharacterVoiceFolder += wstrCharacterId;
	wstrCharacterVoiceFolder += ToBookId(wstrCharacterId, sceneNumber);
	wstrCharacterVoiceFolder += L"_";
	wstrCharacterVoiceFolder += wstrCharacterId;
	wstrCharacterVoiceFolder += L"_r18";

	wstrAudioFolderPath = std::move(wstrCharacterVoiceFolder);

	return true;
}
/* ---------- Example ----------
 * Atlas path: /advscene/resources/advscene/texture/character/r18_scenes/r18_10166_2
 * To
 * Book path: /advscene/scenarioexcel/character/10166
 *------------------------------*/
bool mnms::DeriveEpisodeJsonPathFromSpineFolderPath(const std::wstring& wstrAtlasFolderPath, std::wstring& wstrEpisodeJsonFilePath)
{
	std::wstring wstrCharacterId = ExtractCharacterIdFromSpineFolderPath(wstrAtlasFolderPath);
	if (wstrCharacterId.empty())return false;
	const wchar_t sceneNumber = wstrAtlasFolderPath.back();

	size_t nPos = wstrAtlasFolderPath.find(L"advscene");
	if (nPos == std::wstring::npos)return false;

	std::wstring wstrBookFolder = wstrAtlasFolderPath.substr(0, nPos);
	wstrBookFolder += L"advscene\\scenarioexcel\\character";

	std::wstring wstrBookJson = FindPathContainingId(wstrBookFolder, wstrCharacterId, nullptr);
	if (wstrBookJson.empty())return false;

	wstrBookJson += L"\\";
	wstrBookJson += wstrCharacterId;
	wstrBookJson += ToBookId(wstrCharacterId, sceneNumber);
	wstrBookJson += L"_R18.book";
	wstrBookJson += g_playerSetting.wstrSceneTextExtension;

	wstrEpisodeJsonFilePath = std::move(wstrBookJson);

	return true;
}
/*脚本読み取り*/
bool mnms::ReadScenarioFile(const std::wstring& wstrFilePath, std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames)
{
	bool bRet = SetVoiceFolderPath(wstrFilePath);
	if (!bRet)return false;

	std::string strScenarioFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
	if (!strScenarioFile.empty())
	{
		std::string strError;
		size_t nCurrentAnimationIndex = 0;

		try
		{
			nlohmann::json nlJson = nlohmann::json::parse(strScenarioFile);

			const nlohmann::json& jData = nlJson.at("importGridList").at(0).at("rows");

			/*項目名は飛ばす*/
			for (size_t i = 1; i < jData.size(); ++i)
			{
				const nlohmann::json& jRow = jData[i].at("strings");
				if (jRow.empty())continue;

				if (jRow.size() > CommandIndex::Arg3)
				{
					const std::string& arg3 = jRow[CommandIndex::Arg3];
					if (arg3 == "layer_R18") /* Spine動作指定 */
					{
						animationNames.emplace_back(jRow[CommandIndex::Arg2]);
						nCurrentAnimationIndex = animationNames.size() - 1;

						/* 対応文章がないので前文章データを上書き */
						if (animationNames.back() == "Finish" && !textData.empty())
						{
							textData.back().nAnimationIndex = nCurrentAnimationIndex;
						}
						continue;
					}
				}
				if (jRow.size() > CommandIndex::Text)
				{
					adv::TextDatum t;

					const std::string& strMsg = jRow[CommandIndex::Text];
					if (strMsg.empty())continue;

					t.wstrText.reserve(128);
					const std::string& strName = jRow[CommandIndex::Arg1];
					if (!strName.empty())
					{
						t.wstrText = win_text::WidenUtf8(strName);
						t.wstrText += L":\n";
					}
					t.wstrText += win_text::WidenUtf8(strMsg);
					EliminateTag(t.wstrText);

					if (jRow.size() > CommandIndex::Voice)
					{
						const std::string& strVoicePath = jRow[CommandIndex::Voice];
						if (!strVoicePath.empty())
						{
							t.wstrVoicePath = VoiceRelativeUrlToFilePath(win_text::WidenUtf8(strVoicePath));
						}
					}

					if (!t.wstrText.empty())
					{
						t.nAnimationIndex = nCurrentAnimationIndex;

						textData.push_back(std::move(t));
					}
				}
			}
		}
		catch (nlohmann::json::exception &e)
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
		/*台本なし・読み取り失敗*/
		std::wstring wstrAudioFolderPath;
		mnms::DeriveAudioFolderPathFromSpineFolderPath(wstrFilePath, wstrAudioFolderPath);

		std::vector<std::wstring> audioFilePaths;
		win_filesystem::CreateFilePathList(wstrAudioFolderPath.c_str(), g_playerSetting.wstrVoiceExtension.c_str(), audioFilePaths);

		for (const auto& audioFilePath : audioFilePaths)
		{
			textData.emplace_back(adv::TextDatum{ L"", audioFilePath });
		}
	}

	return !textData.empty();
}
/*描画除外判定関数*/
bool mnms::IsSlotToBeLeftOut(const char* szSlotName, size_t nSlotNameLength)
{
	if (szSlotName != nullptr)
	{
		constexpr const char* const ExclusionSlots[] = { "Mask", "White" };
		for (const auto& ExclusionSlot : ExclusionSlots)
		{
			if (strstr(szSlotName, ExclusionSlot) != nullptr)
			{
				return true;
			}
		}
	}

	return false;
}

unsigned int mnms::GetDisplayRefreshRate()
{
	DEVMODE sDevMode{};
	::EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &sDevMode);
	return sDevMode.dmDisplayFrequency;
}
