#ifndef SFML_SPINE_CPP_H_
#define SFML_SPINE_CPP_H_

#include <spine/spine.h>
#include <SFML/Graphics.hpp>

class CSfmlSpineDrawer : public sf::Drawable
{
public:
	CSfmlSpineDrawer(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pStateData = nullptr);
	~CSfmlSpineDrawer();

	spine::Skeleton* skeleton = nullptr;
	spine::AnimationState* animationState = nullptr;

	bool isAlphaPremultiplied = true;
	bool isToForceBlendModeNormal = false;

	void Update(float fDelta);
	virtual void draw(sf::RenderTarget& renderTarget, sf::RenderStates renderStates) const;

	void SetLeaveOutList(spine::Vector<spine::String>& list);
	void SetLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { m_pLeaveOutCallback = pFunc; }

	sf::FloatRect GetBoundingBox() const;
private:
	bool m_hasOwnAnimationStateData = false;

	mutable spine::SkeletonClipping m_clipper;

	mutable spine::Vector<float> m_worldVertices;
	mutable sf::VertexArray m_sfmlVertices;
	/*SFML does not have indices.*/
	mutable spine::Vector<unsigned short> m_quadIndices;
	
	mutable spine::Vector<spine::String> m_leaveOutList;
	bool IsToBeLeftOut(const spine::String &slotName) const;
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;
};

class CSfmlTextureLoader : public spine::TextureLoader
{
public:
	virtual void load(spine::AtlasPage& page, const spine::String& path);

	virtual void unload(void* texture);
};

#endif //!SFML_SPINE_CPP_H_
