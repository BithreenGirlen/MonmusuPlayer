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

	void GetSpineList(const std::wstring& folderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths);
	bool DeriveVoiceFolderPathFromSpineFolderPath(const std::wstring& sceneSpineFolderPath, std::wstring& sceneVoiceFolderPath);
	bool DeriveScenarioFilePathFromSpineFolderPath(const std::wstring& sceneSpinesFolderPath, std::wstring& sceneScenarioFilePath);

	bool ReadScenarioFile(const std::wstring& scenarioFilePath, std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames);

	bool IsSlotToBeLeftOut(const char* szSlotName, size_t nSlotNameLength);

	unsigned int GetDisplayRefreshRate();
}

#endif // !MNMS_H_

