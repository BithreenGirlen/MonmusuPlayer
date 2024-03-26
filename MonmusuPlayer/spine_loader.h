#ifndef SPINE_LOADER_H_
#define SPINE_LOADER_H_

#include <string>
#include <memory>

#include <spine/spine.h>

namespace spine_loader
{
	std::shared_ptr<spine::SkeletonData> readTextSkeletonFromFile(const spine::String& filename, spine::Atlas* atlas, float scale);
	std::shared_ptr<spine::SkeletonData> readBinarySkeletonFromFile(const spine::String& filename, spine::Atlas* atlas, float scale);

	std::shared_ptr<spine::SkeletonData> readTextSkeletonFromMemory(const std::string& skeleton, spine::Atlas* atlas, float scale);
	std::shared_ptr<spine::SkeletonData> readBinarySkeletonFromMemory(const std::string& skeleton, spine::Atlas* atlas, float scale);
}

#endif
