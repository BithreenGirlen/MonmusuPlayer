

#include "spine-sfml.h"

namespace spine
{
	SpineExtension* getDefaultExtension() {
		return new DefaultSpineExtension();
	}
}

CSfmlSpineDrawable::CSfmlSpineDrawable(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData)
{
	spine::Bone::setYDown(true);

	/*sf::VertexArray seems not to have reserve-like method.*/
	sfmlVertices.setPrimitiveType(sf::PrimitiveType::Triangles);

	skeleton = new(__FILE__, __LINE__) spine::Skeleton(pSkeletonData);

	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = new(__FILE__, __LINE__) spine::AnimationStateData(pSkeletonData);
		m_bHasOwnAnimationStateData = true;
	}

	state = new(__FILE__, __LINE__) spine::AnimationState(pAnimationStateData);

	m_quadIndices.add(0);
	m_quadIndices.add(1);
	m_quadIndices.add(2);
	m_quadIndices.add(2);
	m_quadIndices.add(3);
	m_quadIndices.add(0);
}

CSfmlSpineDrawable::~CSfmlSpineDrawable()
{
	if (state != nullptr)
	{
		if (m_bHasOwnAnimationStateData)
		{
			delete state->getData();
		}

		delete state;
	}
	if (skeleton != nullptr)
	{
		delete skeleton;
	}
}

void CSfmlSpineDrawable::Update(float fDelta)
{
	if (skeleton != nullptr && state != nullptr)
	{
		skeleton->update(fDelta);
		state->update(fDelta * timeScale);
		state->apply(*skeleton);
		skeleton->updateWorldTransform();
	}
}

void CSfmlSpineDrawable::draw(sf::RenderTarget& renderTarget, sf::RenderStates renderStates) const
{
	if (skeleton == nullptr || state == nullptr)return;

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
		int verticesCount = 0;
		spine::Vector<float>* pAttachmentUvs = nullptr;

		spine::Vector<unsigned short>* pIndices = nullptr;
		int indicesCount = 0;

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
			/*Fetch texture handle stored in AltasPage*/
			pSfmlTexture = (sf::Texture*)((spine::AtlasRegion*)pRegionAttachment->getRendererObject())->page->getRendererObject();

			m_worldVertices.setSize(8, 0);
			/*Depends on spine's version whether the first argument is slot or bone.*/
			pRegionAttachment->computeWorldVertices(slot.getBone(), m_worldVertices, 0, 2);
			verticesCount = 4;
			pAttachmentUvs = &pRegionAttachment->getUVs();
			pIndices = &m_quadIndices;
			indicesCount = 6;
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
			pSfmlTexture = (sf::Texture*)((spine::AtlasRegion*)pMeshAttachment->getRendererObject())->page->getRendererObject();

			m_worldVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
			pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), m_worldVertices, 0, 2);
			verticesCount = static_cast<int>(pMeshAttachment->getWorldVerticesLength() / 2);
			pAttachmentUvs = &pMeshAttachment->getUVs();
			pIndices = &pMeshAttachment->getTriangles();
			indicesCount = static_cast<int>(pIndices->size());
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
			verticesCount = static_cast<int>(m_clipper.getClippedVertices().size() / 2);
			pAttachmentUvs = &m_clipper.getClippedUVs();
			pIndices = &m_clipper.getClippedTriangles();
			indicesCount = static_cast<int>(m_clipper.getClippedTriangles().size());
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

		sfmlVertices.clear();
		/*
		* SFML does not support indexed drawing.
		* This requires two tasks to user (namely SFML library user).
		* 
		* 1. Map index to vertex when adding.
		* 2. Multiply alpha to colours if necessary. 
		* 
		*/
		for (int ii = 0; ii < indicesCount; ++ii)
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
			sfmlVertices.append(sfmlVertex);
		}

		sf::BlendMode sfmlBlendMode;
		switch (slot.getData().getBlendMode())
		{
		case spine::BlendMode_Additive:
			sfmlBlendMode.colorSrcFactor = m_bAlphaPremultiplied ? sf::BlendMode::Factor::One : sf::BlendMode::Factor::SrcAlpha;
			sfmlBlendMode.colorDstFactor = sf::BlendMode::Factor::One;
			sfmlBlendMode.colorEquation = sf::BlendMode::Equation::Add;
			sfmlBlendMode.alphaSrcFactor = sf::BlendMode::Factor::One;
			sfmlBlendMode.alphaDstFactor = sf::BlendMode::Factor::One;
			sfmlBlendMode.alphaEquation = sf::BlendMode::Equation::Add;
			break;
		case spine::BlendMode_Multiply:
			sfmlBlendMode.colorSrcFactor = sf::BlendMode::Factor::DstColor;
			sfmlBlendMode.colorDstFactor = sf::BlendMode::Factor::OneMinusSrcAlpha;
			sfmlBlendMode.colorEquation = sf::BlendMode::Equation::Add;
			sfmlBlendMode.alphaSrcFactor = sf::BlendMode::Factor::OneMinusSrcAlpha;
			sfmlBlendMode.alphaDstFactor = sf::BlendMode::Factor::OneMinusSrcAlpha;
			sfmlBlendMode.alphaEquation = sf::BlendMode::Equation::Add;
			break;
		case spine::BlendMode_Screen:
			sfmlBlendMode.colorSrcFactor = sf::BlendMode::Factor::One;
			sfmlBlendMode.colorDstFactor = sf::BlendMode::Factor::OneMinusSrcAlpha;
			sfmlBlendMode.colorEquation = sf::BlendMode::Equation::Add;
			sfmlBlendMode.alphaSrcFactor = sf::BlendMode::Factor::OneMinusSrcColor;
			sfmlBlendMode.alphaDstFactor = sf::BlendMode::Factor::OneMinusSrcColor;
			sfmlBlendMode.alphaEquation = sf::BlendMode::Equation::Add;
			break;
		default:
			sfmlBlendMode.colorSrcFactor = m_bAlphaPremultiplied ? sf::BlendMode::Factor::One : sf::BlendMode::SrcAlpha;
			sfmlBlendMode.colorDstFactor = sf::BlendMode::Factor::OneMinusSrcAlpha;
			sfmlBlendMode.colorEquation = sf::BlendMode::Equation::Add;
			sfmlBlendMode.alphaSrcFactor = sf::BlendMode::Factor::One;
			sfmlBlendMode.alphaDstFactor = sf::BlendMode::Factor::OneMinusSrcAlpha;
			sfmlBlendMode.alphaEquation = sf::BlendMode::Equation::Add;
			break;
		}

		renderStates.blendMode = sfmlBlendMode;
		renderStates.texture = pSfmlTexture;
		renderTarget.draw(sfmlVertices, renderStates);
		m_clipper.clipEnd(slot);
	}
	m_clipper.clipEnd();
}

void CSfmlSpineDrawable::SetLeaveOutList(const std::vector<std::string>& list)
{
	/*There are some slots having mask or nuisance effect; exclude them from rendering.*/
	m_leaveOutList.clear();
	for (const auto& name : list)
	{
		m_leaveOutList.add(name.c_str());
	}
}

bool CSfmlSpineDrawable::IsToBeLeftOut(const spine::String &slotName) const
{
	/*The comparison method depends on what should be excluded; the precise matching or just containing.*/
	for (size_t i = 0; i < m_leaveOutList.size(); ++i)
	{
		if (strstr(slotName.buffer(), m_leaveOutList[i].buffer()) != nullptr)return true;
	}
	return false;
}

void CSfmlTextureLoader::load(spine::AtlasPage& page, const spine::String& path) {
	sf::Texture* texture = new sf::Texture();
	if (!texture->loadFromFile(path.buffer())) return;

	if (page.magFilter == spine::TextureFilter_Linear) texture->setSmooth(true);
	if (page.uWrap == spine::TextureWrap_Repeat && page.vWrap == spine::TextureWrap_Repeat) texture->setRepeated(true);

	page.setRendererObject(texture);
	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
	//sf::Vector2u size = texture->getSize();
	//page.width = size.x;
	//page.height = size.y;
}

void CSfmlTextureLoader::unload(void* texture)
{
	delete (sf::Texture*)texture;
}
