#ifndef ADV_H_
#define ADV_H_

#include <string>

namespace adv
{
	struct TextDatum
	{
		std::wstring wstrText;
		std::wstring wstrVoicePath;

		size_t nAnimationIndex = 0;
	};
}
#endif // !ADV_H_
