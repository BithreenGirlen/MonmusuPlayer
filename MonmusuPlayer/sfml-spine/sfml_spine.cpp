

#include "sfml_spine.h"

namespace spine
{
	SpineExtension* getDefaultExtension()
	{
		return new DefaultSpineExtension();
	}
}

static sf::BlendMode g_sfmlBlendModeNormalPma = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha);
static sf::BlendMode g_sfmlBlendModeAddPma = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::One);
static sf::BlendMode g_sfmlBlendModeScreen = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcColor);
static sf::BlendMode g_sfmlBlendModeMultiply = sf::BlendMode
(
	sf::BlendMode::Factor::DstColor,
	sf::BlendMode::Factor::OneMinusSrcAlpha,
	sf::BlendMode::Equation::Add,
	sf::BlendMode::Factor::One,
	sf::BlendMode::Factor::OneMinusSrcAlpha,
	sf::BlendMode::Equation::Add
);

CSfmlSpineDrawer::CSfmlSpineDrawer(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData)
{
	spine::Bone::setYDown(true);

	/*sf::VertexArray seems not to have reserve-like method.*/
	m_sfmlVertices.setPrimitiveType(sf::PrimitiveType::Triangles);

	skeleton = new spine::Skeleton(pSkeletonData);

	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = new spine::AnimationStateData(pSkeletonData);
		m_hasOwnAnimationStateData = true;
	}

	animationState = new spine::AnimationState(pAnimationStateData);

	m_quadIndices.add(0);
	m_quadIndices.add(1);
	m_quadIndices.add(2);
	m_quadIndices.add(2);
	m_quadIndices.add(3);
	m_quadIndices.add(0);
}

CSfmlSpineDrawer::~CSfmlSpineDrawer()
{
	if (animationState != nullptr)
	{
		if (m_hasOwnAnimationStateData)
		{
			delete animationState->getData();
		}

		delete animationState;
	}
	if (skeleton != nullptr)
	{
		delete skeleton;
	}
}

void CSfmlSpineDrawer::Update(float fDelta)
{
	if (skeleton != nullptr && animationState != nullptr)
	{
#ifndef SPINE_4_1_OR_LATER
		skeleton->update(fDelta);
#endif
		animationState->update(fDelta);
		animationState->apply(*skeleton);
#ifdef SPINE_4_2_OR_LATER
		skeleton->update(fDelta);
		skeleton->updateWorldTransform(spine::Physics::Physics_Update);
#else
		skeleton->updateWorldTransform();
#endif
	}
}

void CSfmlSpineDrawer::draw(sf::RenderTarget& renderTarget, sf::RenderStates renderStates) const
{
	if (skeleton == nullptr || animationState == nullptr)return;

	if (skeleton->getColor().a == 0) return;

	for (unsigned i = 0; i < skeleton->getSlots().size(); ++i)
	{
		spine::Slot& slot = *skeleton->getDrawOrder()[i];
		spine::Attachment* pAttachment = slot.getAttachment();

		if (pAttachment == nullptr || slot.getColor().a == 0 || !slot.getBone().isActive())
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		if (IsToBeLeftOut(slot.getData().getName()))
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		spine::Vector<float>* pVertices = &m_worldVertices;
		spine::Vector<float>* pAttachmentUvs = nullptr;
		spine::Vector<unsigned short>* pIndices = nullptr;

		spine::Color* pAttachmentColor = nullptr;

		sf::Texture* pSfmlTexture = nullptr;

		if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
		{
			spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;
			pAttachmentColor = &pRegionAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}

			m_worldVertices.setSize(8, 0);
#ifdef SPINE_4_1_OR_LATER
			pRegionAttachment->computeWorldVertices(slot, m_worldVertices, 0, 2);
#else
			pRegionAttachment->computeWorldVertices(slot.getBone(), m_worldVertices, 0, 2);
#endif
			pAttachmentUvs = &pRegionAttachment->getUVs();
			pIndices = &m_quadIndices;

#ifdef SPINE_4_1_OR_LATER
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pRegionAttachment->getRegion());

			isAlphaPremultiplied = pAtlasRegion->page->pma;
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->rendererObject);
#else
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pRegionAttachment->getRendererObject());
#ifdef SPINE_4_0
			isAlphaPremultiplied = pAtlasRegion->page->pma;
#endif
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->page->getRendererObject());
#endif
		}
		else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
		{
			spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;
			pAttachmentColor = &pMeshAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}

			m_worldVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
			pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), m_worldVertices, 0, 2);

			pAttachmentUvs = &pMeshAttachment->getUVs();
			pIndices = &pMeshAttachment->getTriangles();

#ifdef SPINE_4_1_OR_LATER
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pMeshAttachment->getRegion());

			isAlphaPremultiplied = pAtlasRegion->page->pma;
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->rendererObject);
#else
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pMeshAttachment->getRendererObject());
#ifdef SPINE_4_0
			isAlphaPremultiplied = pAtlasRegion->page->pma;
#endif
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->page->getRendererObject());
#endif
		}
		else if (pAttachment->getRTTI().isExactly(spine::ClippingAttachment::rtti))
		{
			spine::ClippingAttachment* clip = (spine::ClippingAttachment*)slot.getAttachment();
			m_clipper.clipStart(slot, clip);
			continue;
		}
		else continue;

		if (m_clipper.isClipping())
		{
			m_clipper.clipTriangles(m_worldVertices, *pIndices, *pAttachmentUvs, 2);
			pVertices = &m_clipper.getClippedVertices();
			pAttachmentUvs = &m_clipper.getClippedUVs();
			pIndices = &m_clipper.getClippedTriangles();
		}

		const spine::Color& skeletonColor = skeleton->getColor();
		const spine::Color& slotColor = slot.getColor();
		const spine::Color tint
		(
			skeletonColor.r * slotColor.r * pAttachmentColor->r,
			skeletonColor.g * slotColor.g * pAttachmentColor->g,
			skeletonColor.b * slotColor.b * pAttachmentColor->b,
			skeletonColor.a * slotColor.a * pAttachmentColor->a
		);
		const sf::Vector2u& textureSize = pSfmlTexture->getSize();

		m_sfmlVertices.clear();
		/*
		* The two tasks are required because SFML does not support indexed drawing.
		* 1. Map index to vertex when adding.
		* 2. Multiply alpha to colours if necessary.
		*/
		for (int ii = 0; ii < pIndices->size(); ++ii)
		{
			sf::Vertex sfmlVertex;

			sfmlVertex.position.x = (*pVertices)[(*pIndices)[ii] * 2LL];
			sfmlVertex.position.y = (*pVertices)[(*pIndices)[ii] * 2LL + 1];

			sfmlVertex.color.r = (sf::Uint8)(tint.r * 255.f * (isAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.g = (sf::Uint8)(tint.g * 255.f * (isAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.b = (sf::Uint8)(tint.b * 255.f * (isAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.a = (sf::Uint8)(tint.a * 255.f);

			sfmlVertex.texCoords.x = (*pAttachmentUvs)[(*pIndices)[ii] * 2LL] * textureSize.x;
			sfmlVertex.texCoords.y = (*pAttachmentUvs)[(*pIndices)[ii] * 2LL + 1] * textureSize.y;

			m_sfmlVertices.append(sfmlVertex);
		}

		sf::BlendMode sfmlBlendMode;
		/*
		* Some slots specify BlendMode_Multiply though, BlendMode_Normal seems better;
		* But in order not to overwrite the slot data, copy to temporary variable when forcing.
		*/
		spine::BlendMode spineBlnedMode = isToForceBlendModeNormal ? spine::BlendMode_Normal : slot.getData().getBlendMode();
		switch (spineBlnedMode)
		{
		case spine::BlendMode_Additive:
			sfmlBlendMode = isAlphaPremultiplied ? g_sfmlBlendModeAddPma : sf::BlendAdd;
			break;
		case spine::BlendMode_Multiply:
			sfmlBlendMode = g_sfmlBlendModeMultiply;
			break;
		case spine::BlendMode_Screen:
			sfmlBlendMode = g_sfmlBlendModeScreen;
			break;
		default:
			sfmlBlendMode = isAlphaPremultiplied ? g_sfmlBlendModeNormalPma : sf::BlendAlpha;
			break;
		}

		renderStates.blendMode = sfmlBlendMode;
		renderStates.texture = pSfmlTexture;
		renderTarget.draw(m_sfmlVertices, renderStates);
		m_clipper.clipEnd(slot);
	}
	m_clipper.clipEnd();
}

void CSfmlSpineDrawer::SetLeaveOutList(spine::Vector<spine::String>& list)
{
	/*There are some slots having mask or nuisance effect; exclude them from rendering.*/
	m_leaveOutList.clearAndAddAll(list);
}

sf::FloatRect CSfmlSpineDrawer::GetBoundingBox() const
{
	sf::FloatRect boundingBox{};

	if (skeleton != nullptr)
	{
		spine::Vector<float> tempVertices;
		skeleton->getBounds(boundingBox.left, boundingBox.top, boundingBox.width, boundingBox.height, tempVertices);
	}

	return boundingBox;
}

bool CSfmlSpineDrawer::IsToBeLeftOut(const spine::String& slotName) const
{
	/*The comparison method depends on what should be excluded; the precise matching or partial one.*/
	if (m_pLeaveOutCallback != nullptr)
	{
		return m_pLeaveOutCallback(slotName.buffer(), slotName.length());
	}
	else
	{
		return m_leaveOutList.contains(slotName);
	}

	return false;
}

void CSfmlTextureLoader::load(spine::AtlasPage& atlasPage, const spine::String& path)
{
	sf::Texture* texture = new sf::Texture();
	if (!texture->loadFromFile(path.buffer()))
	{
		delete texture;
		return;
	}

	if (atlasPage.magFilter == spine::TextureFilter_Linear) texture->setSmooth(true);
	if (atlasPage.uWrap == spine::TextureWrap_Repeat && atlasPage.vWrap == spine::TextureWrap_Repeat) texture->setRepeated(true);

	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
	if (atlasPage.width == 0 || atlasPage.height == 0)
	{
		sf::Vector2u size = texture->getSize();
		atlasPage.width = size.x;
		atlasPage.height = size.y;
	}

#ifdef SPINE_4_1_OR_LATER
	atlasPage.texture = texture;
#else
	atlasPage.setRendererObject(texture);
#endif
}

void CSfmlTextureLoader::unload(void* texture)
{
	delete static_cast<sf::Texture*>(texture);
}
