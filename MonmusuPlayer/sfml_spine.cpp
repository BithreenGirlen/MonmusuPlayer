

#include "sfml_spine.h"

namespace spine
{
	/* When will this be deleted? */
	SpineExtension* getDefaultExtension()
	{
		return new DefaultSpineExtension();
	}
}

CSfmlSpineDrawer::CSfmlSpineDrawer(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData)
{
	spine::Bone::setYDown(true);

	/*sf::VertexArray seems not to have reserve-like method.*/
	m_sfmlVertices.setPrimitiveType(sf::PrimitiveType::Triangles);

	skeleton = new(__FILE__, __LINE__) spine::Skeleton(pSkeletonData);

	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = new(__FILE__, __LINE__) spine::AnimationStateData(pSkeletonData);
		m_bHasOwnAnimationStateData = true;
	}

	animationState = new(__FILE__, __LINE__) spine::AnimationState(pAnimationStateData);

	m_quadIndices.add(0);
	m_quadIndices.add(1);
	m_quadIndices.add(2);
	m_quadIndices.add(2);
	m_quadIndices.add(3);
	m_quadIndices.add(0);

	m_sfmlBlendModeNormalPma = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha);
	m_sfmlBlendModeAddPma = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::One);
	m_sfmlBlendModeScreen = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcColor);
	m_sfmlBlendModeMultiply = sf::BlendMode
	(
		sf::BlendMode::Factor::DstColor,
		sf::BlendMode::Factor::OneMinusSrcAlpha,
		sf::BlendMode::Equation::Add,
		sf::BlendMode::Factor::Zero,
		sf::BlendMode::Factor::One,
		sf::BlendMode::Equation::Add
	);
}

CSfmlSpineDrawer::~CSfmlSpineDrawer()
{
	if (animationState != nullptr)
	{
		if (m_bHasOwnAnimationStateData)
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
		animationState->update(fDelta * timeScale);
		animationState->apply(*skeleton);
		skeleton->updateWorldTransform();
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

			/*Fetch texture handle stored in AltasPage*/
#ifdef SPINE_4_1_OR_LATER
			pSfmlTexture = (sf::Texture*)((spine::AtlasRegion*)pRegionAttachment->getRegion())->rendererObject;
#else
			pSfmlTexture = (sf::Texture*)((spine::AtlasRegion*)pRegionAttachment->getRendererObject())->page->getRendererObject();
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

			/*Fetch texture handle stored in AltasPage*/
#ifdef SPINE_4_1_OR_LATER
			pSfmlTexture = (sf::Texture*)((spine::AtlasRegion*)pMeshAttachment->getRegion())->rendererObject;
#else
			pSfmlTexture = (sf::Texture*)((spine::AtlasRegion*)pMeshAttachment->getRendererObject())->page->getRendererObject();
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
		spine::Color tint
		(
			skeletonColor.r * slotColor.r * pAttachmentColor->r,
			skeletonColor.g * slotColor.g * pAttachmentColor->g,
			skeletonColor.b * slotColor.b * pAttachmentColor->b,
			skeletonColor.a * slotColor.a * pAttachmentColor->a
		);
		sf::Vector2u sfmlSize = pSfmlTexture->getSize();

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
			sfmlVertex.color.r = (sf::Uint8)(tint.r * 255.f * (m_bAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.g = (sf::Uint8)(tint.g * 255.f * (m_bAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.b = (sf::Uint8)(tint.b * 255.f * (m_bAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.a = (sf::Uint8)(tint.a * 255.f);
			sfmlVertex.texCoords.x = (*pAttachmentUvs)[(*pIndices)[ii] * 2LL] * sfmlSize.x;
			sfmlVertex.texCoords.y = (*pAttachmentUvs)[(*pIndices)[ii] * 2LL + 1] * sfmlSize.y;
			m_sfmlVertices.append(sfmlVertex);
		}

		sf::BlendMode sfmlBlendMode;
		/*
		* Some slots specify BlendMode_Multiply though, BlendMode_Normal seems better;
		* But in order not to overwrite the slot data, copy to temporary variable when forcing.
		*/
		spine::BlendMode spineBlnedMode = slot.getData().getBlendMode();
		if (m_bForceBlendModeNormal)spineBlnedMode = spine::BlendMode_Normal;
		switch (spineBlnedMode)
		{
		case spine::BlendMode_Additive:
			sfmlBlendMode = m_bAlphaPremultiplied ? m_sfmlBlendModeAddPma : sf::BlendAdd;
			break;
		case spine::BlendMode_Multiply:
			sfmlBlendMode = m_sfmlBlendModeMultiply;
			break;
		case spine::BlendMode_Screen:
			sfmlBlendMode = m_sfmlBlendModeScreen;
			break;
		default:
			sfmlBlendMode = m_bAlphaPremultiplied ? m_sfmlBlendModeNormalPma : sf::BlendAlpha;
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
	m_leaveOutList.clear();
	for (size_t i = 0; i < list.size(); ++i)
	{
		m_leaveOutList.add(list[i].buffer());
	}
}

bool CSfmlSpineDrawer::IsToBeLeftOut(const spine::String &slotName) const
{
	/*The comparison method depends on what should be excluded; the precise matching or just containing.*/
	if (pLeaveOutCallback != nullptr)
	{
		return pLeaveOutCallback(slotName.buffer(), slotName.length());
	}
	else
	{
		for (size_t i = 0; i < m_leaveOutList.size(); ++i)
		{
			if (strcmp(slotName.buffer(), m_leaveOutList[i].buffer()) == 0)return true;
		}
	}

	return false;
}

void CSfmlTextureLoader::load(spine::AtlasPage& page, const spine::String& path)
{
	sf::Texture* texture = new sf::Texture();
	if (!texture->loadFromFile(path.buffer()))
	{
		delete texture;
		return;
	}

	if (page.magFilter == spine::TextureFilter_Linear) texture->setSmooth(true);
	if (page.uWrap == spine::TextureWrap_Repeat && page.vWrap == spine::TextureWrap_Repeat) texture->setRepeated(true);

	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
	if (page.width == 0 || page.height == 0)
	{
		sf::Vector2u size = texture->getSize();
		page.width = size.x;
		page.height = size.y;
	}

#ifdef SPINE_4_1_OR_LATER
	page.texture = texture;
#else
	page.setRendererObject(texture);
#endif
}

void CSfmlTextureLoader::unload(void* texture)
{
	delete (sf::Texture*)texture;
}
