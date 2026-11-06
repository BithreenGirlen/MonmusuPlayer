#ifndef SFML_SPINE_CPP_H_
#define SFML_SPINE_CPP_H_

#include <spine/spine.h>
#include <SFML/Graphics.hpp>

class CSfmlSpineDrawable : public sf::Drawable
{
public:
	CSfmlSpineDrawable(spine::SkeletonData* pSkeletonData);
	~CSfmlSpineDrawable();

	spine::Skeleton* skeleton() const noexcept;
	spine::AnimationState* animationState() const noexcept;

	void premultiplyAlpha(bool premultiplied) noexcept;
	bool isAlphaPremultiplied() const noexcept;

	void forceBlendModeNormal(bool toForce) noexcept;
	bool isBlendModeNormalForced() const noexcept;

	void setPause(bool paused) noexcept;
	bool isPaused() const noexcept;

	void setVisibility(bool visible) noexcept;
	bool isVisible() const noexcept;

	/// @brief How physics to be updated and applied.
	enum class Physics : unsigned char
	{
		None = 0, /* Do not update nor apply physics */
		Reset, /* Reset physics to the current pose */
		Update, /* Update and apply phisics. */
		Pose /* Apply physics, but do not update. */
	};

	/// @brief Physics mode; valid only on  Spine 4.2 and later
	void setPhysics(Physics physics) noexcept;
	Physics getPhysics() const noexcept;

	/// @brief Add animation time and update world transform.
	/// @remark Even if it is paused or 0.0f is passed, world transform will be updated.
	void update(float fDelta);
	void draw(sf::RenderTarget& renderTarget, sf::RenderStates renderStates) const override;

	void setLeaveOutList(spine::Vector<spine::String>& list);
	void setLeaveOutCallback(bool (*pFunc)(const char*, size_t)) noexcept;

	sf::FloatRect getBoundingBox();
	sf::FloatRect getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found = nullptr);
private:
	mutable bool m_isAlphaPremultiplied = true;
	bool m_isToForceBlendModeNormal = false;
	bool m_isVisible = true;
	bool m_isPaused = false;
	Physics m_physics = Physics::Update;

	spine::Skeleton* m_skeleton = nullptr;
	spine::AnimationState* m_animationState = nullptr;

	mutable spine::SkeletonClipping m_clipper;

	mutable spine::Vector<float> m_worldVertices;
	mutable sf::VertexArray m_sfmlVertices;

	/// @brief A buffer to be used to calculate bounding box.
	spine::Vector<float> m_tempVertices;
	
	mutable spine::Vector<spine::String> m_leaveOutList;
	bool isSlotToBeLeftOut(const spine::String &slotName) const;
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;
};

class CSfmlTextureLoader : public spine::TextureLoader
{
public:
	void load(spine::AtlasPage& page, const spine::String& path) override;
	void unload(void* texture) override;

	/// @brief Blend-mode-multiply and blend-mode-screen are allowed only on pre-multiplied texture.
	void enableConversionToPma(bool toEnable) noexcept;
	bool isConversionToPmaEnabled() const noexcept;

	void enableGeneratingMipmap(bool toGenerate) noexcept;
	bool isMipmapToBeGenerated() const noexcept;

	/// @brief Set a load callback; *pOutImage should be casted to sf::Image*
	void setTextureLoadCallback(void (*pFunc)(void* pUserDatum, const char* textureFilePath, size_t filePathLength, void* pOutImage), void* pUserDatum);
private:
	/* AtlasPage has pma field since spine 4.0 */
#if defined(SPINE_40) || defined(SPINE_41) || defined (SPINE_42)
	/// @brief Though atlas page field being non-pma, the texture has already been converted to PMA via external tool
	bool m_wasAlreadyConvertedToPma = false;
#else /* Spine 3.8 */
	bool m_toConvertToPma = false;
#endif
	bool m_toGenerateMipmap = false;

	void (*m_pTextureLoadCallback)(void* pUserDatum, const char* textureFilePath, size_t filePathLength, void* pOutImage) = nullptr;
	void* m_pCallbackUserDatum = nullptr;
};

#endif //!SFML_SPINE_CPP_H_
