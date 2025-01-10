#ifndef MNMS_H_
#define MNMS_H_

#include <string>
#include <vector>

#include "adv.h"

namespace mnms
{
	bool InitialiseSetting();
	const std::string& GetFontFilePath();
	const bool IsSkelBinary();

	void GetSpineList(const std::wstring& wstrFolderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths);
	bool DeriveAudioFolderPathFromSpineFolderPath(const std::wstring& wstrAtlasFolderPath, std::wstring& wstrAudioFolderPath);
	bool DeriveEpisodeJsonPathFromSpineFolderPath(const std::wstring& wstrAtlasFolderPath, std::wstring& wstrEpisodeJsonFilePath);

	bool ReadScenarioFile(const std::wstring& wstrFilePath, std::vector<adv::TextDatum>& textData);

	bool IsSlotToBeLeftOut(const char* szSlotName, size_t nSlotNameLength);
}

#endif // !MNMS_H_

