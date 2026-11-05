#ifndef SPINE_LOADER_H_
#define SPINE_LOADER_H_

#include <memory>

#include <spine/Atlas.h>
#include <spine/SkeletonData.h>

namespace spine_loader
{
	std::shared_ptr<spine::Atlas> CreateAtlasFromFile(const char* filePath, spine::TextureLoader* textureLoader);
	std::shared_ptr<spine::Atlas> CreateAtlasFromMemory(const char* atlasFileData, int atlasFileDataLength, const char* textureDirectory, spine::TextureLoader* textureLoader);

	std::shared_ptr<spine::SkeletonData> ReadTextSkeletonFromFile(const char* filePath, spine::Atlas* atlas);
	std::shared_ptr<spine::SkeletonData> ReadBinarySkeletonFromFile(const char* filePath, spine::Atlas* atlas);

	std::shared_ptr<spine::SkeletonData> ReadTextSkeletonFromMemory(const char* skeletonJson, spine::Atlas* atlas);
	std::shared_ptr<spine::SkeletonData> ReadBinarySkeletonFromMemory(const unsigned char* skeletonBinary, int skeletonLength, spine::Atlas* atlas);
}

#endif
