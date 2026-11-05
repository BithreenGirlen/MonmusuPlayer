

#include "spine_loader.h"

#include <spine/SkeletonJson.h>
#include <spine/SkeletonBinary.h>
#include <spine/TextureLoader.h>


std::shared_ptr<spine::Atlas> spine_loader::CreateAtlasFromFile(const char* filePath, spine::TextureLoader* textureLoader)
{
	return std::make_shared<spine::Atlas>(filePath, textureLoader);
}

std::shared_ptr<spine::Atlas> spine_loader::CreateAtlasFromMemory(const char* atlasFileData, int atlasFileDataLength, const char* textureDirectory, spine::TextureLoader* textureLoader)
{
	return std::make_shared<spine::Atlas>(atlasFileData, atlasFileDataLength, textureDirectory, textureLoader);
}

std::shared_ptr<spine::SkeletonData> spine_loader::ReadTextSkeletonFromFile(const char* filePath, spine::Atlas* atlas)
{
	spine::SkeletonJson json(atlas);
	json.setScale(1.f);

	spine::SkeletonData* skeletonData = json.readSkeletonDataFile(filePath);
	if (skeletonData == nullptr)
	{
		return nullptr;
	}

	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

std::shared_ptr<spine::SkeletonData> spine_loader::ReadBinarySkeletonFromFile(const char* filePath, spine::Atlas* atlas)
{
	spine::SkeletonBinary binary(atlas);
	binary.setScale(1.f);

	spine::SkeletonData* skeletonData = binary.readSkeletonDataFile(filePath);
	if (!skeletonData)
	{
		return nullptr;
	}

	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

std::shared_ptr<spine::SkeletonData> spine_loader::ReadTextSkeletonFromMemory(const char* skeletonJson, spine::Atlas* atlas)
{
	spine::SkeletonJson json(atlas);
	json.setScale(1.f);

	spine::SkeletonData* skeletonData = json.readSkeletonData(skeletonJson);
	if (!skeletonData)
	{
		return nullptr;
	}

	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

std::shared_ptr<spine::SkeletonData> spine_loader::ReadBinarySkeletonFromMemory(const unsigned char* skeletonBinary, int skeletonLength, spine::Atlas* atlas)
{
	spine::SkeletonBinary binary(atlas);
	binary.setScale(1.f);

	spine::SkeletonData* skeletonData = binary.readSkeletonData(skeletonBinary, skeletonLength);
	if (!skeletonData)
	{
		return nullptr;
	}

	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}
