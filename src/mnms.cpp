
#include <Windows.h>

#include <algorithm>

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
		std::wstring settingFilePath = std::wstring(win_filesystem::GetCurrentProcessPath()).append(L"setting.txt");
		std::string settingFile = win_filesystem::LoadFileAsString(settingFilePath.c_str());
		if (settingFile.empty())return false;

		try
		{
			nlohmann::json nlJson = nlohmann::json::parse(settingFile);
			const nlohmann::json& jExtension = nlJson.at("extensions");
			const std::string* pStr = nullptr;

			pStr = jExtension.at("atlas").get<const std::string*>();
			if (pStr != nullptr)
			{
				playerSetting.wstrAtlasExtension = win_text::WidenUtf8(pStr->data(), static_cast<int>(pStr->size()));
			}

			pStr = jExtension.at("skel").get<const std::string*>();
			if (pStr != nullptr)
			{
				playerSetting.wstrSkelExtension = win_text::WidenUtf8(pStr->data(), static_cast<int>(pStr->size()));
			}

			pStr = jExtension.at("voice").get<const std::string*>();
			if (pStr != nullptr)
			{
				playerSetting.wstrVoiceExtension = win_text::WidenUtf8(pStr->data(), static_cast<int>(pStr->size()));
			}

			pStr = jExtension.at("sceneText").get<const std::string*>();
			if (pStr != nullptr)
			{
				playerSetting.wstrSceneTextExtension = win_text::WidenUtf8(pStr->data(), static_cast<int>(pStr->size()));
			}

			pStr = nlJson.at("fontPath").get<const std::string*>();
			if (pStr != nullptr)
			{
				playerSetting.strFontFilePath.assign(*pStr);
			}
		}
		catch (const nlohmann::json::exception& e)
		{
			win_dialogue::ShowErrorMessageBox("Setting error", e.what());

			return false;
		}

		return true;
	}

	struct CommandIndex abstract final
	{
		enum : uint8_t
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

	/// @brief 台本ファイル経路から音声階層導出
	static std::wstring DeriveVoiceDirectoryFtomScenarioFilePath(const std::wstring& scenerioFilePath)
	{
		size_t nPos = scenerioFilePath.rfind(L"advscene\\");
		if (nPos == std::wstring::npos)return {};

		return scenerioFilePath.substr(0, nPos).append(L"advscene\\resources\\advscene\\sound\\voice\\");
	}

	/* 人物ID抽出 */
	static std::wstring ExtractCharacterIdFromSpineFolderPath(const std::wstring& sceneSpineFolderPath)
	{
		size_t nPos1 = sceneSpineFolderPath.find_last_of(L"\\/");
		if (nPos1 == std::wstring::npos)return {};
		++nPos1;

		static constexpr std::wstring_view s_sceneType = L"r18_";
		size_t nPos2 = sceneSpineFolderPath.find(s_sceneType, nPos1);
		if (nPos2 == std::wstring::npos)return {};
		nPos2 += s_sceneType.length();

		nPos1 = sceneSpineFolderPath.find(L'_', nPos2);
		if (nPos1 == std::wstring::npos)return {};

		return sceneSpineFolderPath.substr(nPos2, nPos1 - nPos2);
	}

	/* ID対応先探索 */
	static std::wstring FindPathContainingId(const std::wstring& folderPath, const std::wstring& characterId, const std::wstring_view fileExtension)
	{
		std::vector<std::wstring> paths;
		win_filesystem::CreateFilePathList(folderPath, fileExtension, paths);
		const auto iter = std::find_if(paths.begin(), paths.end(),
			[&characterId](const std::wstring& path)
			-> bool
			{
				return path.find(characterId) != std::wstring::npos;
			});
		if (iter == paths.cend())return {};

		return *iter;
	}

	static std::wstring ToBookId(const std::wstring& characterId, const wchar_t sceneNumber)
	{
		if (!characterId.empty() && characterId[0] == L'3')
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

	template <typename CharType>
	void EliminateTagInPlace(std::basic_string<CharType>& str)
	{
		size_t nWritten = 0;
		int iCount = 0;
		for (const auto& c : str)
		{
			if (c == CharType('<'))
			{
				++iCount;
				continue;
			}
			else if (c == CharType('>'))
			{
				--iCount;
				continue;
			}
			if (iCount == 0)
			{
				str[nWritten++] = c;
			}
		}

		str.resize(nWritten);
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

bool mnms::IsSkelBinary()
{
	constexpr const std::wstring_view binarySkelCandidates[] =
	{
		L".skel", L".bin"
	};

	for (const auto& binarySkelCandidate : binarySkelCandidates)
	{
		if (g_playerSetting.wstrSkelExtension.find(binarySkelCandidate) != std::wstring::npos)
		{
			return true;
		}
	}

	return false;
}
/*描画素材一覧取得*/
void mnms::GetSpineList(const std::wstring& sceneSpineFolderPath, std::vector<std::string>& atlasFilePaths, std::vector<std::string>& skelFilePaths)
{
	const bool isAtlasLonger = g_playerSetting.wstrAtlasExtension.size() > g_playerSetting.wstrSkelExtension.size();

	const std::wstring& longerExtesion = isAtlasLonger ? g_playerSetting.wstrAtlasExtension : g_playerSetting.wstrSkelExtension;
	const std::wstring& shorterExtension = isAtlasLonger ? g_playerSetting.wstrSkelExtension : g_playerSetting.wstrAtlasExtension;
	std::vector<std::string>& longerPaths = isAtlasLonger ? atlasFilePaths : skelFilePaths;
	std::vector<std::string>& shorterPaths = isAtlasLonger ? skelFilePaths : atlasFilePaths;

	std::vector<std::wstring> filePaths;
	win_filesystem::CreateFilePathList(sceneSpineFolderPath, L"*", filePaths);

	for (const auto& filePath : filePaths)
	{
		if (filePath.ends_with(longerExtesion))
		{
			longerPaths.emplace_back(win_text::NarrowUtf8(filePath));
		}
		else if (filePath.ends_with(shorterExtension))
		{
			shorterPaths.emplace_back(win_text::NarrowUtf8(filePath));
		}
	}
}
/* ---------- Example ----------
 * Scene Spine path: /texture/character/r18_scenes/r18_10166_2
 * To
 * Scene voice path: /sound/voice/ch_10166/r18/10166b1_10166_r18
 *------------------------------*/
bool mnms::DeriveVoiceFolderPathFromSpineFolderPath(const std::wstring& sceneSpineFolderPath, std::wstring& audioFolderPath)
{
	std::wstring characterId = ExtractCharacterIdFromSpineFolderPath(sceneSpineFolderPath);
	if (characterId.empty())return false;
	const wchar_t sceneNumber = sceneSpineFolderPath.back();

	size_t nPos = sceneSpineFolderPath.find(L"texture");
	if (nPos == std::wstring::npos)return false;

	std::wstring voiceParentFolderPath = sceneSpineFolderPath.substr(0, nPos).append(L"sound\\voice");

	std::wstring characterVoiceFolderPath = FindPathContainingId(voiceParentFolderPath, characterId, {});
	if (characterVoiceFolderPath.empty())return false;

	characterVoiceFolderPath += L"\\r18\\";
	characterVoiceFolderPath += characterId;
	characterVoiceFolderPath += ToBookId(characterId, sceneNumber);
	characterVoiceFolderPath += L"_";
	characterVoiceFolderPath += characterId;
	characterVoiceFolderPath += L"_r18";

	audioFolderPath = std::move(characterVoiceFolderPath);

	return true;
}
/* ---------- Example ----------
 * Scene Spine path: /advscene/resources/advscene/texture/character/r18_scenes/r18_10166_2
 * To
 * Scene script path: /advscene/scenarioexcel/character/10166/10166b1_R18.book.json
 *------------------------------*/
bool mnms::DeriveScenarioFilePathFromSpineFolderPath(const std::wstring& sceneSpineFolderPath, std::wstring& scenarioFilePath)
{
	std::wstring characterId = ExtractCharacterIdFromSpineFolderPath(sceneSpineFolderPath);
	if (characterId.empty())return false;
	const wchar_t sceneNumber = sceneSpineFolderPath.back();

	size_t nPos = sceneSpineFolderPath.find(L"advscene");
	if (nPos == std::wstring::npos)return false;

	std::wstring sceneScenarioParentFolderPath = sceneSpineFolderPath.substr(0, nPos).append(L"advscene\\scenarioexcel\\character");

	std::wstring characterScenarioFilePath = FindPathContainingId(sceneScenarioParentFolderPath, characterId, {});
	if (characterScenarioFilePath.empty())return false;

	characterScenarioFilePath += L"\\";
	characterScenarioFilePath += characterId;
	characterScenarioFilePath += ToBookId(characterId, sceneNumber);
	characterScenarioFilePath += L"_R18.book";
	characterScenarioFilePath += g_playerSetting.wstrSceneTextExtension;

	scenarioFilePath = std::move(characterScenarioFilePath);

	return true;
}
/*脚本読み取り*/
bool mnms::ReadScenarioFile(const std::wstring& scenarioFilePath, std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames)
{
	std::wstring voiceDirectory = DeriveVoiceDirectoryFtomScenarioFilePath(scenarioFilePath);
	if (voiceDirectory.empty())return false;

	std::string scenarioFile = win_filesystem::LoadFileAsString(scenarioFilePath.c_str());
	if (!scenarioFile.empty())
	{
		size_t nCurrentAnimationIndex = 0;

		try
		{
			nlohmann::json nlJson = nlohmann::json::parse(scenarioFile);

			const nlohmann::json& jData = nlJson.at("importGridList").at(0).at("rows");

			/*項目名は飛ばす*/
			for (size_t i = 1; i < jData.size(); ++i)
			{
				const nlohmann::json& jRow = jData[i].at("strings");
				if (jRow.empty())continue;

				if (jRow.size() > CommandIndex::Arg3)
				{
					std::string_view arg3 = jRow[CommandIndex::Arg3];
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

					std::string_view sceneText = jRow[CommandIndex::Text];
					if (sceneText.empty())continue;

					std::string_view characterName = jRow[CommandIndex::Arg1];
					if (!characterName.empty())
					{
						t.wstrText.assign(win_text::WidenUtf8(characterName.data(), static_cast<int>(characterName.size())));
						t.wstrText += L":\n";
					}
					t.wstrText += win_text::WidenUtf8(sceneText.data(), static_cast<int>(sceneText.size()));
					EliminateTagInPlace(t.wstrText);

					if (jRow.size() > CommandIndex::Voice)
					{
						std::string_view voiceFilePath = jRow[CommandIndex::Voice];
						if (!voiceFilePath.empty())
						{
							/* ch_XXXXX/R18/XXXXXB1_XXXXX_R18/XXXXXB0_XXXXX_YYY の書式でで高々48文字 */
							wchar_t pathBuffer[128]{};
							static constexpr size_t kPathBufferSize = std::size(pathBuffer);
							int nWritten = win_text::WidenUtf8InBuffer(voiceFilePath.data(), static_cast<int>(voiceFilePath.size()), pathBuffer, kPathBufferSize);
							if (nWritten != 0)
							{
								std::replace(pathBuffer, pathBuffer + nWritten, L'/', L'\\');
								t.wstrVoicePath
									.assign(voiceDirectory)
									.append(pathBuffer, nWritten)
									.append(g_playerSetting.wstrVoiceExtension);
							}
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
		catch (const nlohmann::json::exception& e)
		{
			win_dialogue::ShowErrorMessageBox("Parse error", e.what());
			return false;
		}
	}

	if (textData.empty()) [[unlikely]]
	{
		/*台本なし・読み取り失敗*/
		std::wstring voiceFolderPath;
		mnms::DeriveVoiceFolderPathFromSpineFolderPath(scenarioFilePath, voiceFolderPath);

		std::vector<std::wstring> voiceFilePaths;
		win_filesystem::CreateFilePathList(voiceFolderPath, g_playerSetting.wstrVoiceExtension, voiceFilePaths);

		for (const auto& voiceFilePath : voiceFilePaths)
		{
			textData.emplace_back(adv::TextDatum{ L"", voiceFilePath });
		}
	}

	return !textData.empty();
}
/*描画除外判定関数*/
bool mnms::IsSlotToBeLeftOut(const char* szSlotName, size_t nSlotNameLength)
{
	bool isToBeLeftOut = false;
	if (szSlotName != nullptr)
	{
		static constexpr std::string_view s_slotNamesToExclude[] =
		{
			"Mask", "WhiteSpot"
		};

		const char* pNameEnd = szSlotName + nSlotNameLength;
		for (const auto& slotNameToExclude : s_slotNamesToExclude)
		{
			const char* pFilterStart = slotNameToExclude.data();
			const char* pFilterEnd = slotNameToExclude.data() + slotNameToExclude.size();

			isToBeLeftOut |= std::search(szSlotName, pNameEnd, pFilterStart, pFilterEnd) != pNameEnd;
		}
	}

	return isToBeLeftOut;
}

unsigned int mnms::GetDisplayRefreshRate()
{
	DEVMODE devMode{};
	::EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
	return devMode.dmDisplayFrequency;
}
