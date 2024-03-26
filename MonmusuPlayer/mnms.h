#ifndef MNMS_H_
#define MNMS_H_

#include <string>
#include <vector>

#include "adv.h"

namespace mnms
{
	void GetSpineList(const std::wstring& wstrFolderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths);
	bool DeriveAudioFolderPathFromSpineFolderPath(const std::wstring& wstrAtlasFolderPath, std::wstring& wstrAudioFolderPath);
	bool DeriveEpisodeJsonPathFromSpineFolderPath(const std::wstring& wstrAtlasFolderPath, std::wstring& wstrEpisodeJsonFilePath);

	bool ReadScenarioFile(const std::wstring& wstrFilePath, std::vector<adv::TextDatum>& textData);
}

#endif // !MNMS_H_

