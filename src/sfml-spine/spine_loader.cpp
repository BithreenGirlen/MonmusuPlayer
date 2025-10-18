

#include "spine_loader.h"

std::shared_ptr<spine::SkeletonData> spine_loader::ReadTextSkeletonFromFile(const char* filePath, spine::Atlas* atlas, float scale)
{
	spine::SkeletonJson json(atlas);
	json.setScale(scale);
	auto skeletonData = json.readSkeletonDataFile(filePath);
	if (!skeletonData)
	{
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

std::shared_ptr<spine::SkeletonData> spine_loader::ReadBinarySkeletonFromFile(const char* filePath, spine::Atlas* atlas, float scale)
{
	spine::SkeletonBinary binary(atlas);
	binary.setScale(scale);
	auto skeletonData = binary.readSkeletonDataFile(filePath);
	if (!skeletonData)
	{
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

std::shared_ptr<spine::SkeletonData> spine_loader::ReadTextSkeletonFromMemory(const char* skeletonJson, spine::Atlas* atlas, float scale)
{
	spine::SkeletonJson json(atlas);
	json.setScale(scale);
	auto skeletonData = json.readSkeletonData(skeletonJson);
	if (!skeletonData)
	{
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

std::shared_ptr<spine::SkeletonData> spine_loader::ReadBinarySkeletonFromMemory(const unsigned char* skeletonBinary, int skeletonLength, spine::Atlas* atlas, float scale)
{
	spine::SkeletonBinary binary(atlas);
	binary.setScale(scale);
	auto skeletonData = binary.readSkeletonData(skeletonBinary, skeletonLength);
	if (!skeletonData)
	{
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}
