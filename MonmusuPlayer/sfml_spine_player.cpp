
#include "sfml_spine_player.h"
#include "spine_loader.h"

CSfmlSpinePlayer::CSfmlSpinePlayer()
{

}

CSfmlSpinePlayer::~CSfmlSpinePlayer()
{

}

bool CSfmlSpinePlayer::SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary)
{
	if (atlasPaths.size() != skelPaths.size())return false;

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths.at(i);
		const std::string& strSkeletonPath = skelPaths.at(i);

		m_atlases.emplace_back(std::make_unique<spine::Atlas>(strAtlasPath.c_str(), &m_textureLoader));

		std::shared_ptr<spine::SkeletonData> skeletonData = bIsBinary ? spine_loader::readBinarySkeletonFromFile(strSkeletonPath.c_str(), m_atlases.back().get(), 1.f) : spine_loader::readTextSkeletonFromFile(strSkeletonPath.c_str(), m_atlases.back().get(), 1.f);
		if (skeletonData == nullptr)return false;

		m_skeletonData.emplace_back(skeletonData);
	}

	if (m_skeletonData.empty())return false;

	m_BaseWindowSize.x = m_skeletonData.at(0).get()->getWidth();
	m_BaseWindowSize.y = m_skeletonData.at(0).get()->getHeight();

	WorkOutDefaultScale();

	return SetupDrawer();
}

bool CSfmlSpinePlayer::SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData.at(i);
		const std::string& strAtlasPath = atlasPaths.at(i);
		const std::string& strSkeletonData = skelData.at(i);

		m_atlases.emplace_back(std::make_unique<spine::Atlas>(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum .size()), strAtlasPath.c_str(), &m_textureLoader));

		std::shared_ptr<spine::SkeletonData> skeletonData = bIsBinary ? spine_loader::readBinarySkeletonFromMemory(strSkeletonData, m_atlases.back().get(), 1.f) : spine_loader::readTextSkeletonFromMemory(strSkeletonData, m_atlases.back().get(), 1.f);
		if (skeletonData == nullptr)return false;

		m_skeletonData.emplace_back(skeletonData);
	}

	if (m_skeletonData.empty())return false;

	m_BaseWindowSize.x = m_skeletonData.at(0).get()->getWidth();
	m_BaseWindowSize.y = m_skeletonData.at(0).get()->getHeight();

	WorkOutDefaultScale();

	return SetupDrawer();
}
/*字体ファイル設定*/
bool CSfmlSpinePlayer::SetFont(const std::string& strFilePath, bool bBold, bool bItalic)
{
	bool bRet = m_font.loadFromFile(strFilePath);
	if (!bRet)return false;

	constexpr float fOutLineThickness = 2.4f;

	m_msgText.setFont(m_font);
	m_msgText.setCharacterSize(64);
	m_msgText.setFillColor(sf::Color::Black);
	m_msgText.setStyle((bBold ? sf::Text::Style::Bold : 0) | (bItalic ? sf::Text::Style::Italic : 0));
	m_msgText.setOutlineThickness(fOutLineThickness);
	m_msgText.setOutlineColor(sf::Color::White);

	return true;
}

void CSfmlSpinePlayer::SetTexts(const std::vector<adv::TextDatum>& textData)
{
	m_textData = textData;

	const auto HasAudio = [](const adv::TextDatum& text)
		-> bool
		{
			return !text.wstrVoicePath.empty();
		};
	const auto iter = std::find_if(m_textData.begin(), m_textData.end(), HasAudio);
	if (iter != m_textData.cend())
	{
		m_pAudioPlayer = std::make_unique<CMfMediaPlayer>(nullptr, 0);
	}
}
/*ウィンドウ表示*/
int CSfmlSpinePlayer::Display(const wchar_t* pwzWindowName)
{
	sf::Vector2i iMouseStartPos;

	bool bOnWindowMove = false;
	bool bSpeedHavingChanged = false;

	int iRet = 0;

	UpdateMessageText();

	m_window = std::make_unique< sf::RenderWindow>(sf::VideoMode(static_cast<unsigned int>(m_BaseWindowSize.x), static_cast<unsigned int>(m_BaseWindowSize.y)), pwzWindowName, sf::Style::None);
	m_window->setPosition(sf::Vector2i(0, 0));
	m_window->setFramerateLimit(0);
	ResetScale();

	sf::Event event;
	sf::Clock deltaClock;
	while (m_window->isOpen())
	{
		while (m_window->pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				m_window->close();
				break;
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					iMouseStartPos.x = event.mouseButton.x;
					iMouseStartPos.y = event.mouseButton.y;
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					/*速度変更完了*/
					if (bSpeedHavingChanged)
					{
						bSpeedHavingChanged = false;
						break;
					}

					/*窓枠移動開始・完了*/
					if (bOnWindowMove || sf::Mouse::isButtonPressed(sf::Mouse::Right))
					{
						bOnWindowMove ^= true;
						break;
					}

					int iX = iMouseStartPos.x - event.mouseButton.x;
					int iY = iMouseStartPos.y - event.mouseButton.y;

					if (iX == 0 && iY == 0)
					{
						ShiftScene();
					}
					else
					{
						MoveViewPoint(iX, iY);
					}
				}
				if (event.mouseButton.button == sf::Mouse::Middle)
				{
					ResetScale();
				}
				break;
			case sf::Event::MouseWheelScrolled:
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					RescaleTime(event.mouseWheelScroll.delta < 0);
					bSpeedHavingChanged = true;
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
				{
					ShiftMessageText(event.mouseWheelScroll.delta < 0);
				}
				else
				{
					RescaleSkeleton(event.mouseWheelScroll.delta < 0);
				}
				break;
			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::Key::C:
					SwitchTextColor();
					break;
				case sf::Keyboard::Key::T:
					m_bTextHidden ^= true;
					break;
				case sf::Keyboard::Key::Escape:
					m_window->close();
					break;
				case sf::Keyboard::Key::PageUp:
					ChangePlaybackRate(true);
					break;
				case sf::Keyboard::Key::PageDown:
					ChangePlaybackRate(false);
					break;
				case sf::Keyboard::Key::Home:
					ResetPlacybackRate();
					break;
				case sf::Keyboard::Key::Up:
					iRet = 2;
					m_window->close();
					break;
				case sf::Keyboard::Key::Down:
					iRet = 1;
					m_window->close();
					break;
				default:
					break;
				}
				break;
			}
		}

		float delta = deltaClock.getElapsedTime().asSeconds();
		deltaClock.restart();
		Redraw(delta);

		CheckTimer();

		if (bOnWindowMove)
		{
			int iPosX = sf::Mouse::getPosition().x - m_window->getSize().x / 2;
			int iPosY = sf::Mouse::getPosition().y - m_window->getSize().y / 2;
			m_window->setPosition(sf::Vector2i(iPosX, iPosY));
		}
	}
	return iRet;
}
/*描画器設定*/
bool CSfmlSpinePlayer::SetupDrawer()
{
	const std::vector<std::string> leaveOutList{ "Mask", "White" };

	for (size_t i = 0; i < m_skeletonData.size(); ++i)
	{
		m_drawables.emplace_back(std::make_shared<CSfmlSpineDrawable>(m_skeletonData.at(i).get()));

		CSfmlSpineDrawable* drawable = m_drawables.at(i).get();
		drawable->timeScale = 1.0f;
		drawable->skeleton->setPosition(m_BaseWindowSize.x / 2, m_BaseWindowSize.y / 2);
		drawable->skeleton->updateWorldTransform();

		drawable->SetLeaveOutList(leaveOutList);

		auto& animations = m_skeletonData.at(i).get()->getAnimations();
		for (size_t ii = 0; ii < animations.size(); ++ii)
		{
			const std::string &strAnimationName = animations[ii]->getName().buffer();
			auto iter = std::find(m_animationNames.begin(), m_animationNames.end(), strAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(strAnimationName);
		}

		auto& skins = m_skeletonData.at(i).get()->getSkins();
		for (size_t ii = 0; ii < skins.size(); ++ii)
		{
			const std::string& strName = skins[ii]->getName().buffer();
			auto iter = std::find(m_skinNames.begin(), m_skinNames.end(), strName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(strName);
		}
	}

	/*寝室再生順*/
	std::vector<std::string> fixedNames = { "Wait", "Normal", "Fast", "Finish", "After" };
	auto IsR18 = [&fixedNames](const std::string& str)
		-> bool
		{
			for (const std::string& fixedName : fixedNames)
			{
				if (strstr(str.c_str(), fixedName.c_str()) != nullptr)return true;
			}
			return false;
		};
	if (m_animationNames.size()  == 6)
	{
		/*静画*/
		fixedNames.push_back("sa");
	}
	if (std::all_of(m_animationNames.begin(), m_animationNames.end(), IsR18))
	{
		m_animationNames = fixedNames;
	}

	if (!m_animationNames.empty())
	{
		for (size_t i = 0; i < m_skeletonData.size(); ++i)
		{
			CSfmlSpineDrawable* drawable = m_drawables.at(i).get();
			drawable->state->setAnimation(0, m_animationNames.at(0).c_str(), true);
		}
	}

	return m_animationNames.size() > 0;
}
/*標準尺度算出*/
void CSfmlSpinePlayer::WorkOutDefaultScale()
{
	if (m_skeletonData.empty())return;

	unsigned int uiSkeletonWidth = static_cast<unsigned int>(m_skeletonData.at(0).get()->getWidth());
	unsigned int uiSkeletonHeight = static_cast<unsigned int>(m_skeletonData.at(0).get()->getHeight());

	unsigned int uiDesktopWidth = sf::VideoMode::getDesktopMode().width;
	unsigned int uiDesktopHeight = sf::VideoMode::getDesktopMode().height;

	if (uiSkeletonWidth > uiDesktopWidth || uiSkeletonHeight > uiDesktopHeight)
	{
		if (uiDesktopWidth > uiDesktopHeight)
		{
			m_fDefaultWindowScale = static_cast<float>(uiDesktopHeight) / uiSkeletonHeight;
			m_fThresholdScale = static_cast<float>(uiDesktopWidth) / uiSkeletonWidth;
		}
		else
		{
			m_fDefaultWindowScale = static_cast<float>(uiDesktopWidth) / uiSkeletonWidth;
			m_fThresholdScale = static_cast<float>(uiDesktopHeight) / uiSkeletonHeight;
		}
		m_fSkeletonScale = m_fDefaultWindowScale;
	}
}
/*拡縮変更*/
void CSfmlSpinePlayer::RescaleSkeleton(bool bUpscale)
{
	constexpr float kfMinScale = 0.25f;
	if (bUpscale)
	{
		m_fSkeletonScale += m_kfScalePortion;
	}
	else
	{
		m_fSkeletonScale -= m_kfScalePortion;
		if (m_fSkeletonScale < kfMinScale)m_fSkeletonScale = kfMinScale;
	}
	UpdateScaletonScale();
}
/*時間尺度変更*/
void CSfmlSpinePlayer::RescaleTime(bool bHasten)
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

	UpdateTimeScale();
}
/*尺度変更適用設定*/
void CSfmlSpinePlayer::UpdateScaletonScale()
{
	float fOffset = m_fSkeletonScale - m_fThresholdScale > 0.f ? m_fSkeletonScale - m_fThresholdScale : 0;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->skeleton->setScaleX(m_fSkeletonScale > 0.99f + fOffset ? m_fSkeletonScale : 1.f + fOffset);
		m_drawables.at(i).get()->skeleton->setScaleY(m_fSkeletonScale > 0.99f + fOffset ? m_fSkeletonScale : 1.f + fOffset);
	}

	unsigned int uiWindowWidthMax = static_cast<unsigned int>(m_BaseWindowSize.x * (m_fSkeletonScale - m_kfScalePortion));
	unsigned int uiWindowHeightMax = static_cast<unsigned int>(m_BaseWindowSize.y * (m_fSkeletonScale - m_kfScalePortion));
	if (uiWindowWidthMax < sf::VideoMode::getDesktopMode().width || uiWindowHeightMax < sf::VideoMode::getDesktopMode().height)
	{
		ResizeWindow();
	}
}
/*速度変更適用*/
void CSfmlSpinePlayer::UpdateTimeScale()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->timeScale = m_fTimeScale;
	}
}
/*速度・尺度・視点初期化*/
void CSfmlSpinePlayer::ResetScale()
{
	m_fTimeScale = 1.0f;
	m_fSkeletonScale = m_fDefaultWindowScale;
	m_iOffset = sf::Vector2i{};

	UpdateScaletonScale();
	UpdateTimeScale();
	MoveViewPoint(0, 0);
	ResizeWindow();
}
/*窓寸法調整*/
void CSfmlSpinePlayer::ResizeWindow()
{
	if (m_window.get() != nullptr)
	{
		m_window->setSize(sf::Vector2u(static_cast<unsigned int>(m_BaseWindowSize.x * m_fSkeletonScale), static_cast<unsigned int>(m_BaseWindowSize.y * m_fSkeletonScale)));
	}
}
/*視点移動*/
void CSfmlSpinePlayer::MoveViewPoint(int iX, int iY)
{
	m_iOffset.x += iX;
	m_iOffset.y += iY;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->skeleton->setPosition(m_BaseWindowSize .x/ 2 - m_iOffset.x, m_BaseWindowSize.y / 2 - m_iOffset.y);
	}
}
/*場面移行*/
void CSfmlSpinePlayer::ShiftScene()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex > m_animationNames.size() - 1)m_nAnimationIndex = 0;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->state->setAnimation(0, m_animationNames.at(m_nAnimationIndex).c_str(), true);
	}
}

void CSfmlSpinePlayer::ShiftSkin(bool bForward)
{
	if (bForward)
	{
		++m_nSkinIndex;
	}
	else
	{
		--m_nSkinIndex;
	}
	if (m_nSkinIndex > m_skinNames.size() - 1)m_nSkinIndex = 0;
	UpdateSkin();
}

void CSfmlSpinePlayer::UpdateSkin()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		spine::Skin *skin = m_skeletonData.at(i).get()->findSkin(m_skinNames.at(m_nSkinIndex).c_str());
		m_drawables.at(i).get()->skeleton->setSkin(skin);
		m_drawables.at(i).get()->skeleton->setSlotsToSetupPose();
	}
}
/*文字色切り替え*/
void CSfmlSpinePlayer::SwitchTextColor()
{
	m_msgText.setFillColor(m_msgText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
	m_msgText.setOutlineColor(m_msgText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
}
/*文章表示位置調整*/
void CSfmlSpinePlayer::AdjustTextPosition()
{
	/*左下表示用*/
	//int iScaletonHeight = static_cast<int>(m_BaseWindowSize.y * m_fSkeletonScale);
	//int iClientHeight = sf::VideoMode::getDesktopMode().height;

	//if (iScaletonHeight < iClientHeight)
	//{
	//	m_msgText.setPosition(sf::Vector2f{ 0, m_BaseWindowSize.y - m_msgText.getGlobalBounds().height });
	//}
	//else
	//{
	//	float fScale = static_cast<float>(iClientHeight) / iScaletonHeight;
	//	m_msgText.setPosition(sf::Vector2f{ 0, m_BaseWindowSize.y * fScale - m_msgText.getGlobalBounds().height });
	//}
}

void CSfmlSpinePlayer::CheckTimer()
{
	constexpr float fAutoPlayInterval = 2.f;
	float fSecond = m_clock.getElapsedTime().asSeconds();
	if (m_pAudioPlayer.get() != nullptr && m_pAudioPlayer.get()->IsEnded() && fSecond > fAutoPlayInterval)
	{
		if (m_nTextIndex < m_textData.size() - 1)
		{
			ShiftMessageText(true);
		}
		else
		{
			m_clock.restart();
		}
	}
}
/*文章移行*/
void CSfmlSpinePlayer::ShiftMessageText(bool bForward)
{
	if (bForward)
	{
		++m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
	}
	else
	{
		--m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
	}
	UpdateMessageText();
}

void CSfmlSpinePlayer::UpdateMessageText()
{
	if (m_textData.empty())return;

	const adv::TextDatum& textDatum = m_textData.at(m_nTextIndex);
	std::wstring wstr = textDatum.wstrText + L"\r\n " + std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
	m_msgText.setString(wstr);

	if (!textDatum.wstrVoicePath.empty())
	{
		if (m_pAudioPlayer.get() != nullptr)
		{
			m_pAudioPlayer->Play(textDatum.wstrVoicePath.c_str());
		}
	}
	m_clock.restart();
}
/*再描画*/
void CSfmlSpinePlayer::Redraw(float fDelta)
{
	if (m_window.get() != nullptr)
	{
		m_window->clear();
		for (size_t i = 0; i < m_drawables.size(); ++i)
		{
			m_drawables.at(i).get()->Update(fDelta);
			m_window->draw(*m_drawables.at(i).get(), sf::RenderStates(sf::BlendAlpha));
		}
		if (!m_bTextHidden)
		{
			m_window->draw(m_msgText);
		}
		m_window->display();
	}
}

void CSfmlSpinePlayer::ChangePlaybackRate(bool bFaster)
{
	if (m_pAudioPlayer.get() != nullptr)
	{
		double dbRate = m_pAudioPlayer.get()->GetCurrentRate();
		constexpr double fRatePortion = 0.1;
		if (bFaster && dbRate < 2.49)
		{
			dbRate += fRatePortion;
		}
		else if (!bFaster && dbRate > 0.51)
		{
			dbRate -= fRatePortion;

		}
		m_pAudioPlayer.get()->SetCurrentRate(dbRate);
	}
}

void CSfmlSpinePlayer::ResetPlacybackRate()
{
	if (m_pAudioPlayer.get() != nullptr)
	{
		m_pAudioPlayer.get()->SetCurrentRate(1.0);
	}
}
