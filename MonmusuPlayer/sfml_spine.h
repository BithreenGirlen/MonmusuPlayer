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
	float timeScale = 1.f;

	void Update(float fDelta);
	virtual void draw(sf::RenderTarget& renderTarget, sf::RenderStates renderStates) const;

	void SwitchPma() { m_bAlphaPremultiplied ^= true; };
	void SwitchBlendModeAdoption() { m_bForceBlendModeNormal ^= true; }

	void SetLeaveOutList(spine::Vector<spine::String>& list);
	void SetLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { pLeaveOutCallback = pFunc; }
private:
	bool m_bHasOwnAnimationStateData = false;
	bool m_bAlphaPremultiplied = true;
	bool m_bForceBlendModeNormal = false;

	mutable spine::SkeletonClipping m_clipper;

	mutable spine::Vector<float> m_worldVertices;
	mutable sf::VertexArray m_sfmlVertices;
	/*SFML does not have indices.*/
	mutable spine::Vector<unsigned short> m_quadIndices;
	
	mutable spine::Vector<spine::String> m_leaveOutList;
	bool IsToBeLeftOut(const spine::String &slotName) const;
	bool (*pLeaveOutCallback)(const char*, size_t) = nullptr;

	sf::BlendMode m_sfmlBlendModeNormalPma;
	sf::BlendMode m_sfmlBlendModeAddPma;
	sf::BlendMode m_sfmlBlendModeScreen;
	sf::BlendMode m_sfmlBlendModeMultiply;
};

class CSfmlTextureLoader : public spine::TextureLoader
{
public:
	virtual void load(spine::AtlasPage& page, const spine::String& path);

	virtual void unload(void* texture);
};

#endif //!SFML_SPINE_CPP_H_
