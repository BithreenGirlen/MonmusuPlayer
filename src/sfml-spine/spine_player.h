#ifndef SPINE_PLAYER_H_
#define SPINE_PLAYER_H_

/* Base-type spine player regardless of rendering library. */

#include <string>
#include <vector>
#include <memory>
#if defined(ALLOW_FORCE_REPLACEMENT)
	#include <unordered_map>
#endif

#include "sfml_spine.h"
using FPoint2 = sf::Vector2f;
using CSpineDrawable = CSfmlSpineDrawable;
using CTextureLoader = CSfmlTextureLoader;

class CSpinePlayer
{
public:
	CSpinePlayer() = default;
	virtual ~CSpinePlayer() = default;

	bool loadSpineFromFile(const std::vector<std::string>& atlasFilePaths, const std::vector<std::string>& skeletonFilePaths, bool isBinarySkel);
	bool loadSpineFromMemory(const std::vector<std::string>& atlasFileData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& skeletonFileData, bool isBinarySkel);

	bool addSpineFromFile(const char* atlasPath, const char* skelPath, bool isBinarySkel);

	size_t getNumberOfSpines() const noexcept;
	bool hasSpineBeenLoaded() const noexcept;

	void update(float fDelta);

	void resetScale();

	void addOffset(int iX, int iY);

	void shiftAnimation();
	void shiftSkin();

	void setAnimationByIndex(size_t nIndex);
	void setAnimationByName(const char* animationName);
	void restartAnimation(bool loop = true);

	void setSkinByIndex(size_t nIndex);
	void setSkinByName(const char* skinName);
	void setupSkin();

	/// @brief To perform PMA blend-mode on vertices or not.
	/// @return false if it were out of range.
	bool premultiplyAlpha(bool premultiplied, size_t nDrawableIndex = 0) noexcept;
	void premultiplyAlphaAll(bool premultiplied) noexcept;
	/// @return current state. If it were out of range, return false.
	bool isAlphaPremultiplied(size_t nDrawableIndex = 0) const noexcept;

	/// @brief To force blend-mode-normal or not.
	/// @return false if it were out of range
	bool forceBlendModeNormal(bool toForce, size_t nDrawableIndex = 0) noexcept;
	void forceBlendModeNormalAll(bool toForce) noexcept;
	bool isBlendModeNormalForced(size_t nDrawableIndex = 0) const noexcept;

	bool setPause(bool paused, size_t nDrawableIndex = 0) noexcept;
	void setPauseAll(bool paused) noexcept;
	bool isPaused(size_t nDrawableIndex = 0) const noexcept;

	bool setVisibility(bool visible, size_t nDrawableIndex = 0) noexcept;
	void setVisibilityAll(bool visible) noexcept;
	bool isVisible(size_t nDrawableIndex = 0) const noexcept;

	bool setPhysics(CSpineDrawable::Physics physics, size_t nDrawableIndex = 0) noexcept;
	void setPhysicsAll(CSpineDrawable::Physics physics) noexcept;
	CSpineDrawable::Physics getPhysics(size_t nDrawableIndex = 0) const noexcept;

	void setDrawOrder(bool reversed) noexcept;
	bool isDrawOrderReversed() const noexcept;

	void enableConversionToPmaOnLoading(bool toEnable) noexcept;
	bool isConversionToPmaOnLoadingEnabled() const noexcept;
	void setTextureLoadCallback(void (*pFunc)(void* pUserDatum, const char* textureFilePath, size_t filePathLength, void* pOutImage), void* pUserDatum) noexcept;

	/// @brief Get animation name actually entried in the track.
	const char* getCurrentAnimationName();
	/// @brief Get animation time actually entried in track.
	/// @param fTrack elapsed time since the track was entried.
	/// @param fLast current timeline position.
	/// @param fStart timeline start position.
	/// @param fEnd timeline end position.
	void getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd);
	void setCurrentAnimationTime(float animationTime);
	float getAnimationDuration(const char* animationName);

	/// @brief Nullptr if none of the skins are set after loading
	const char* getCurrentSkinName();

	const std::vector<std::string>& getSlotNames() const noexcept;
	const std::vector<std::string>& getSkinNames() const noexcept;
	const std::vector<std::string>& getAnimationNames() const noexcept;

	void mixSkins(const std::vector<std::string>& skinNames);
	void addAnimationTracks(const std::vector<std::string>& animationNames, bool loop = false);
	void mixAnimations(const char* fadeOutAnimationName, const char* fadeInAnimationName, float mixTime);
	void clearMixedAnimation();

	void setSlotsToExclude(const std::vector<std::string>& slotNames);
	void setSlotExcludeCallback(bool (*pFunc)(const char*, size_t));

#if defined(ALLOW_FORCE_REPLACEMENT)
	/// @brief Searches slots having multiple attachments. If each slot is associated with only single attachment, returns empty.
	/// @return slot name as key and attachment names as values.
	std::unordered_map<std::string, std::vector<std::string>> getSlotNamesWithTheirAttachments();
	bool replaceAttachment(const char* szSlotName, const char* szAttachmentName);
#endif

	FPoint2 getBaseSize() const noexcept;
	void setBaseSize(float fWidth, float fHeight);
	void resetBaseSize();

	FPoint2 getOffset() const noexcept;
	void setOffset(float fX, float fY) noexcept;

	float getSkeletonScale() const noexcept;
	void setSkeletonScale(float fScale);

	float getCanvasScale() const noexcept;
	void setCanvasScale(float fScale) noexcept;

	float getTimeScale() const noexcept;
	void setTimeScale(float fTimeScale) noexcept;
protected:
	enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

	CTextureLoader m_textureLoader;
	std::vector<std::shared_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;
	std::vector<std::unique_ptr<CSpineDrawable>> m_drawables;

	FPoint2 m_fBaseSize = FPoint2{ kBaseWidth, kBaseHeight };

	float m_fDefaultScale = 1.f;
	FPoint2 m_fDefaultOffset{};

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	float m_fCanvasScale = 1.f;
	FPoint2 m_fOffset{};

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	std::vector<std::string> m_slotNames;

	bool m_isDrawOrderReversed = false;

	void clearDrawables();
	bool addDrawable(spine::SkeletonData* pSkeletonData);
	bool setupDrawables();

	virtual void workOutDefaultScale() = 0;
	virtual bool workOutDefaultSizeAndOffset() = 0;

	void updatePosition();

	void clearAnimationTracks();
};


#endif // !SPINE_PLAYER_H_
