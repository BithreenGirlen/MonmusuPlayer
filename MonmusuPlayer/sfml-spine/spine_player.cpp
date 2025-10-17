
#include "spine_player.h"
#include "spine_loader.h"

CSpinePlayer::CSpinePlayer()
{

}

CSpinePlayer::~CSpinePlayer()
{

}

/*ファイル取り込み*/
bool CSpinePlayer::LoadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonPath = skelPaths[i];

		std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(strAtlasPath.c_str(), &m_textureLoader);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
			spine_loader::ReadBinarySkeletonFromFile(strSkeletonPath.c_str(), atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromFile(strSkeletonPath.c_str(), atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return SetupDrawer();
}
/*メモリ取り込み*/
bool CSpinePlayer::LoadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool isBinarySkel)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData[i];
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonData = skelData[i];

		std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum.size()), strAtlasPath.c_str(), &m_textureLoader);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
			spine_loader::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>(strSkeletonData.data()), static_cast<int>(strSkeletonData.size()), atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromMemory(strSkeletonData.data(), atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return SetupDrawer();
}

size_t CSpinePlayer::GetNumberOfSpines() const
{
	return m_drawables.size();
}

bool CSpinePlayer::HasSpineBeenLoaded() const
{
	return !m_drawables.empty();
}
/*状態更新*/
void CSpinePlayer::Update(float fDelta)
{
	for (const auto& drawable : m_drawables)
	{
		drawable->Update(fDelta * m_fTimeScale);
	}
}
/*拡縮変更*/
void CSpinePlayer::RescaleSkeleton(bool bUpscale)
{
	if (bUpscale)
	{
		m_fSkeletonScale += kfScalePortion;
	}
	else
	{
		m_fSkeletonScale -= kfScalePortion;
		if (m_fSkeletonScale < kfMinScale)m_fSkeletonScale = kfMinScale;
	}
}

void CSpinePlayer::RescaleCanvas(bool bUpscale)
{
	if (bUpscale)
	{
		m_fCanvasScale += kfScalePortion;
	}
	else
	{
		m_fCanvasScale -= kfScalePortion;
		if (m_fCanvasScale < kfMinScale)m_fCanvasScale = kfMinScale;
	}
}
/*時間尺度変更*/
void CSpinePlayer::RescaleTime(bool bHasten)
{
	constexpr float kfTimeScalePortion = 0.05f;
	if (bHasten)
	{
		m_fTimeScale += kfTimeScalePortion;
	}
	else
	{
		m_fTimeScale -= kfTimeScalePortion;
	}
	if (m_fTimeScale < 0.f)m_fTimeScale = 0.f;
}
/*速度・尺度・視点初期化*/
void CSpinePlayer::ResetScale()
{
	m_fTimeScale = 1.0f;

	m_fSkeletonScale = m_fDefaultScale;
	m_fCanvasScale = m_fDefaultScale;

	m_fOffset = m_fDefaultOffset;

	UpdatePosition();
}
/*視点移動*/
void CSpinePlayer::MoveViewPoint(int iX, int iY)
{
	m_fOffset.x += iX / m_fSkeletonScale;
	m_fOffset.y += iY / m_fSkeletonScale;
	UpdatePosition();
}
/*動作移行*/
void CSpinePlayer::ShiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex >= m_animationNames.size())m_nAnimationIndex = 0;

	ClearAnimationTracks();
	RestartAnimation();
}
/*装い移行*/
void CSpinePlayer::ShiftSkin()
{
	if (m_skinNames.empty())return;

	++m_nSkinIndex;
	if (m_nSkinIndex >= m_skinNames.size())m_nSkinIndex = 0;

	const char* szSkinName = m_skinNames[m_nSkinIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spine::Skin* skin = pDrawable->skeleton->getData()->findSkin(szSkinName);
		if (skin != nullptr)
		{
			pDrawable->skeleton->setSkin(skin);
			pDrawable->skeleton->setSlotsToSetupPose();
		}
	}
}

void CSpinePlayer::SetAnimationByIndex(size_t nIndex)
{
	if (nIndex < m_animationNames.size())
	{
		m_nAnimationIndex = nIndex;
		RestartAnimation();
	}
}

void CSpinePlayer::SetAnimationByName(const char* szAnimationName)
{
	if (szAnimationName != nullptr)
	{
		const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), szAnimationName);
		if (iter != m_animationNames.cend())
		{
			m_nAnimationIndex = std::distance(m_animationNames.begin(), iter);
			RestartAnimation();
		}
	}
}
/*動作適用*/
void CSpinePlayer::RestartAnimation()
{
	if (m_nAnimationIndex >= m_animationNames.size())return;
	const char* szAnimationName = m_animationNames[m_nAnimationIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spine::Animation* pAnimation = pDrawable->skeleton->getData()->findAnimation(szAnimationName);
		if (pAnimation != nullptr)
		{
			pDrawable->animationState->setAnimation(0, pAnimation->getName(), true);
		}
	}
}
/*乗算済み透過度有効・無効切り替え*/
void CSpinePlayer::TogglePma()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->isAlphaPremultiplied ^= true;
	}
}
/*槽溝指定合成方法採択可否*/
void CSpinePlayer::ToggleBlendModeAdoption()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->isToForceBlendModeNormal ^= true;
	}
}
/*描画順切り替え*/
void CSpinePlayer::ToggleDrawOrder()
{
	m_bDrawOrderReversed ^= true;
}

const char* CSpinePlayer::GetCurrentAnimationName()
{
	for (const auto& pDrawable : m_drawables)
	{
		auto& tracks = pDrawable->animationState->getTracks();
		for (size_t i = 0; i < tracks.size(); ++i)
		{
			spine::Animation* pAnimation = tracks[i]->getAnimation();
			if (pAnimation != nullptr)
			{
				return pAnimation->getName().buffer();
			}
		}
	}

	return nullptr;
}

void CSpinePlayer::GetCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	for (const auto& pDrawable : m_drawables)
	{
		auto& tracks = pDrawable->animationState->getTracks();
		for (size_t i = 0; i < tracks.size(); ++i)
		{
			spine::Animation* pAnimation = tracks[i]->getAnimation();
			if (pAnimation != nullptr)
			{
				if (fTrack != nullptr)*fTrack = tracks[i]->getTrackTime();
				if (fLast != nullptr)*fLast = tracks[i]->getAnimationLast();
				if (fStart != nullptr)*fStart = tracks[i]->getAnimationStart();
				if (fEnd != nullptr)*fEnd = tracks[i]->getAnimationEnd();

				return;
			}
		}
	}
}
/*槽溝名称引き渡し*/
std::vector<std::string> CSpinePlayer::GetSlotNames()
{
	std::vector<std::string> slotNames;
	for (const auto& skeletonDatum : m_skeletonData)
	{
		auto& slots = skeletonDatum->getSlots();
		for (size_t ii = 0; ii < slots.size(); ++ii)
		{
			const char* szName = slots[ii]->getName().buffer();
			const auto iter = std::find(slotNames.begin(), slotNames.end(), szName);
			if (iter == slotNames.cend())slotNames.push_back(szName);
		}
	}

	return slotNames;
}
/*装い名称引き渡し*/
const std::vector<std::string>& CSpinePlayer::GetSkinNames() const
{
	return m_skinNames;
}
/*動作名称引き渡し*/
const std::vector<std::string>& CSpinePlayer::GetAnimationNames() const
{
	return m_animationNames;
}
/*描画除外リスト設定*/
void CSpinePlayer::SetSlotsToExclude(const std::vector<std::string>& slotNames)
{
	spine::Vector<spine::String> leaveOutList;
	for (const auto& slotName : slotNames)
	{
		leaveOutList.add(slotName.c_str());
	}

	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->SetLeaveOutList(leaveOutList);
	}
}
/*装い合成*/
void CSpinePlayer::MixSkins(const std::vector<std::string>& skinNames)
{
	if (m_nSkinIndex >= m_skinNames.size())return;
	const auto& currentSkinName = m_skinNames[m_nSkinIndex];

	for (const auto& pDrawble : m_drawables)
	{
		spine::Skin* skinToSet = pDrawble->skeleton->getData()->findSkin(currentSkinName.c_str());
		if (skinToSet == nullptr)continue;

		for (const auto& skinName : skinNames)
		{
			if (currentSkinName != skinName)
			{
				spine::Skin* skinToAdd = pDrawble->skeleton->getData()->findSkin(skinName.c_str());
				if (skinToAdd != nullptr)
				{
					skinToSet->addSkin(skinToAdd);
				}
			}
		}
		pDrawble->skeleton->setSkin(skinToSet);
		pDrawble->skeleton->setSlotsToSetupPose();
	}
}
/*動作合成*/
void CSpinePlayer::MixAnimations(const std::vector<std::string>& animationNames)
{
	ClearAnimationTracks();

	if (m_nAnimationIndex >= m_animationNames.size())return;
	const auto& currentAnimationName = m_animationNames[m_nAnimationIndex];

	for (const auto& pDrawable : m_drawables)
	{
		if (pDrawable->skeleton->getData()->findAnimation(currentAnimationName.c_str()) == nullptr)continue;

		int iTrack = 1;
		for (const auto& animationName : animationNames)
		{
			if (animationName != currentAnimationName)
			{
				spine::Animation* animation = pDrawable->skeleton->getData()->findAnimation(animationName.c_str());
				if (animation != nullptr)
				{
					pDrawable->animationState->addAnimation(iTrack, animation, false, 0.f);
					++iTrack;
				}
			}
		}
	}
}
/*描画除外是否関数登録*/
void CSpinePlayer::SetSlotExclusionCallback(bool(*pFunc)(const char*, size_t))
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->SetLeaveOutCallback(pFunc);
	}
}
/*寸法受け渡し*/
FPoint2 CSpinePlayer::GetBaseSize() const
{
	return m_fBaseSize;
}
/*尺度受け渡し*/
float CSpinePlayer::GetCanvasScale() const
{
	return m_fCanvasScale;
}

void CSpinePlayer::SetZoom(float fZoom)
{
	m_fSkeletonScale = m_fCanvasScale * fZoom;
}
/*消去*/
void CSpinePlayer::ClearDrawables()
{
	m_drawables.clear();
	m_atlases.clear();
	m_skeletonData.clear();

	m_animationNames.clear();
	m_nAnimationIndex = 0;

	m_skinNames.clear();
	m_nSkinIndex = 0;
}
/*描画器設定*/
bool CSpinePlayer::SetupDrawer()
{
	WorkOutDefaultSize();
	WorkOutDefaultScale();

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		auto pDrawable = std::make_shared<CSpineDrawable>(pSkeletonDatum.get());
		if (pDrawable.get() == nullptr)continue;

		pDrawable->skeleton->setPosition(m_fBaseSize.x / 2, m_fBaseSize.y / 2);
		pDrawable->Update(0.f);

		m_drawables.push_back(std::move(pDrawable));

		auto& animations = pSkeletonDatum->getAnimations();
		for (size_t i = 0; i < animations.size(); ++i)
		{
			const char* szAnimationName = animations[i]->getName().buffer();
			if (szAnimationName == nullptr)continue;

			const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), szAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(szAnimationName);
		}

		auto& skins = pSkeletonDatum->getSkins();
		for (size_t i = 0; i < skins.size(); ++i)
		{
			const char* szSkinName = skins[i]->getName().buffer();
			if (szSkinName == nullptr)continue;

			const auto& iter = std::find(m_skinNames.begin(), m_skinNames.end(), szSkinName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(szSkinName);
		}

	}

	WorkOutDefaultOffset();

	RestartAnimation();

	ResetScale();

	return m_animationNames.size() > 0;
}
/*基準寸法・位置算出*/
void CSpinePlayer::WorkOutDefaultSize()
{
	if (m_skeletonData.empty())return;

	float fMaxSize = 0.f;
	const auto CompareDimention = [this, &fMaxSize](float fWidth, float fHeight)
		-> bool
		{
			if (fWidth > 0.f && fHeight > 0.f && fWidth * fHeight > fMaxSize)
			{
				m_fBaseSize.x = fWidth;
				m_fBaseSize.y = fHeight;
				fMaxSize = fWidth * fHeight;
				return true;
			}

			return false;
		};

	for (const auto& pSkeletonData : m_skeletonData)
	{
		if (pSkeletonData.get()->getWidth() > 0 && pSkeletonData.get()->getHeight())
		{
			CompareDimention(pSkeletonData.get()->getWidth(), pSkeletonData.get()->getHeight());
		}
		else
		{
			/*Why spine::Skin lacks searching methods based on its own spine::Vector<spine::Attachment*>?*/
			const auto FindDefaultSkinAttachment = [&pSkeletonData]()
				-> spine::Attachment*
				{
					spine::Skin::AttachmentMap::Entries attachmentMapEntries = pSkeletonData.get()->getDefaultSkin()->getAttachments();
					for (; attachmentMapEntries.hasNext();)
					{
						spine::Skin::AttachmentMap::Entry attachmentMapEntry = attachmentMapEntries.next();
						if (attachmentMapEntry._slotIndex == 0)
						{
							return attachmentMapEntry._attachment;
						}
					}
					return nullptr;
				};

			spine::Attachment* pAttachment = FindDefaultSkinAttachment();
			if (pAttachment == nullptr)continue;

			if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
			{
				spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;

				CompareDimention(pRegionAttachment->getWidth() * pRegionAttachment->getScaleX(), pRegionAttachment->getHeight() * pRegionAttachment->getScaleY());
			}
			else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
			{
				spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;

				spine::SlotData* pSlotData = pSkeletonData.get()->findSlot(pAttachment->getName());

				float fScaleX = pSlotData != nullptr ? pSlotData->getBoneData().getScaleX() : 1.f;
				float fScaleY = pSlotData != nullptr ? pSlotData->getBoneData().getScaleY() : 1.f;

				CompareDimention(pMeshAttachment->getWidth() * fScaleX, pMeshAttachment->getHeight() * fScaleY);
			}
		}
	}
}
/*位置適用*/
void CSpinePlayer::UpdatePosition()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->skeleton->setPosition(m_fBaseSize.x / 2 - m_fOffset.x, m_fBaseSize.y / 2 - m_fOffset.y);
	}
}
/*合成動作消去*/
void CSpinePlayer::ClearAnimationTracks()
{
	for (const auto& pDrawable : m_drawables)
	{
		const auto& trackEntry = pDrawable->animationState->getTracks();
		for (size_t iTrack = 1; iTrack < trackEntry.size(); ++iTrack)
		{
			pDrawable->animationState->setEmptyAnimation(iTrack, 0.f);
		}
	}
}