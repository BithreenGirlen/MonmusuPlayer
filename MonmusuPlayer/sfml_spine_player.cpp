
#include "sfml_spine_player.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "winmm.lib")

#ifdef  _DEBUG
#pragma comment(lib, "sfml-system-d.lib")
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#else
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#endif // _DEBUG

#include "win_media_player.h"


std::shared_ptr<spine::SkeletonData> readSkeletonJsonData(const spine::String& filename, spine::Atlas* atlas, float scale) 
{
	spine::SkeletonJson json(atlas);
	json.setScale(scale);
	auto skeletonData = json.readSkeletonDataFile(filename);
	if (!skeletonData) 
	{
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

std::shared_ptr<spine::SkeletonData> readSkeletonBinaryData(const char* filename, spine::Atlas* atlas, float scale) 
{
	spine::SkeletonBinary binary(atlas);
	binary.setScale(scale);
	auto skeletonData = binary.readSkeletonDataFile(filename);
	if (!skeletonData) 
	{
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

CSfmlSpinePlayer::CSfmlSpinePlayer()
{

}

CSfmlSpinePlayer::~CSfmlSpinePlayer()
{

}

bool CSfmlSpinePlayer::SetSpine(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	Clear();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths.at(i);
		const std::string& strSkeletonPath = skelPaths.at(i);

		m_atlases.emplace_back(std::make_unique<spine::Atlas>(strAtlasPath.c_str(), &m_textureLoader));

		std::shared_ptr<spine::SkeletonData> skeletonData = bIsBinary ? readSkeletonBinaryData(strSkeletonPath.c_str(), m_atlases.back().get(), 1.f) : readSkeletonJsonData(strSkeletonPath.c_str(), m_atlases.back().get(), 1.f);
		if (skeletonData == nullptr)return false;

		m_skeletonData.emplace_back(skeletonData);
	}

	if (m_skeletonData.empty())return false;

	m_fMaxWidth = m_skeletonData.at(0).get()->getWidth();
	m_fMaxHeight = m_skeletonData.at(0).get()->getHeight();

	WorkOutDefaultSize();

	return SetupDrawer();
}
/*音声ファイル設定*/
void CSfmlSpinePlayer::SetAudios(std::vector<std::wstring>& filePaths)
{
	m_audio_files = filePaths;
}
/*ウィンドウ表示*/
int CSfmlSpinePlayer::Display()
{
	sf::Vector2i iMouseStartPos;

	bool bOnWindowMove = false;
	bool bSpeedHavingChanged = false;

	int iRet = 0;
	m_window = std::make_unique< sf::RenderWindow>(sf::VideoMode(static_cast<unsigned int>(m_fMaxWidth), static_cast<unsigned int>(m_fMaxHeight)), "Monmusu spine player", sf::Style::None);
	m_window->setPosition(sf::Vector2i(0, 0));
	m_window->setFramerateLimit(0);
	ResetScale();

	/*The media player is based on Microsoft Media Foundation because SFML does not support .m4a file.*/

	std::unique_ptr<CMediaPlayer> pMediaPlayer = std::make_unique<CMediaPlayer>(m_window->getSystemHandle());
	pMediaPlayer->SetFiles(m_audio_files);
	double dbAudioRate = 1.0;

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
					/*速度変更*/
					if (event.mouseWheelScroll.delta < 0)
					{
						m_fTimeScale += 0.05f;
					}
					else
					{
						m_fTimeScale -= 0.05f;
					}
					RescaleTime();
					bSpeedHavingChanged = true;
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
				{
					/*音声送り・戻し*/
					if (pMediaPlayer.get() != nullptr)
					{
						if (event.mouseWheelScroll.delta < 0)
						{
							pMediaPlayer->Next();
						}
						else
						{
							pMediaPlayer->Back();
						}
					}
				}
				else
				{
					/*拡縮変更*/
					if (event.mouseWheelScroll.delta < 0)
					{
						m_fSkeletonScale += 0.025f;
					}
					else
					{
						m_fSkeletonScale -= 0.025f;
						if (m_fSkeletonScale < 0.25f)m_fSkeletonScale = 0.25f;
					}
					RescaleSkeleton();
				}
				break;
			case sf::Event::KeyReleased:
				if (event.key.code == sf::Keyboard::Key::Escape)
				{
					m_window->close();
				}
				if (event.key.code == sf::Keyboard::Key::Up)
				{
					iRet = 2;
					m_window->close();
				}
				if (event.key.code == sf::Keyboard::Key::Down)
				{
					iRet = 1;
					m_window->close();
				}
				if (event.key.code == sf::Keyboard::Key::PageUp)
				{
					if (dbAudioRate < 2.41)
					{
						dbAudioRate += 0.1;
					}
					if (pMediaPlayer.get() != nullptr)pMediaPlayer->SetCurrentRate(dbAudioRate);
				}
				if (event.key.code == sf::Keyboard::Key::PageDown)
				{
					if (dbAudioRate > 0.59)
					{
						dbAudioRate -= 0.1;
					}
					if (pMediaPlayer.get() != nullptr)pMediaPlayer->SetCurrentRate(dbAudioRate);
				}
				if (event.key.code == sf::Keyboard::Key::Home)
				{
					dbAudioRate = 1.0;
					if (pMediaPlayer.get() != nullptr)pMediaPlayer->SetCurrentRate(dbAudioRate);
				}
				break;
			}
		}

		float delta = deltaClock.getElapsedTime().asSeconds();
		deltaClock.restart();
		Redraw(delta);

		if (bOnWindowMove)
		{
			int iPosX = sf::Mouse::getPosition().x - m_window->getSize().x / 2;
			int iPosY = sf::Mouse::getPosition().y - m_window->getSize().y / 2;
			m_window->setPosition(sf::Vector2i(iPosX, iPosY));
		}
	}
	return iRet;
}
/*消去*/
void CSfmlSpinePlayer::Clear()
{
	m_atlases.clear();
	m_skeletonData.clear();
	m_animationNames.clear();
}
/*描画器設定*/
bool CSfmlSpinePlayer::SetupDrawer()
{
	const std::vector<std::string> blendScreenList{ "Breath", "Mist", "Bless", "Eff", "Smoke", "Toiki", "Steam", "Moya"};
	const std::vector<std::string> blendMultiplyList{"Cheek"};
	const std::vector<std::string> leaveOutList{ "Mask", "White"};

	for (size_t i = 0; i < m_skeletonData.size(); ++i)
	{
		m_drawables.emplace_back(std::make_shared<spine::SkeletonDrawable>(m_skeletonData.at(i).get()));

		spine::SkeletonDrawable* drawable = m_drawables.at(i).get();
		drawable->timeScale = 1.0f;
		drawable->skeleton->setPosition(m_fMaxWidth / 2, m_fMaxHeight / 2);
		drawable->skeleton->updateWorldTransform();

		drawable->SetLeaveOutList(leaveOutList);

		/*合成方法指定。反転合成は透過度に関係なく、乗算合成は透過度を見て変更。*/
		drawable->SetBlendMultiplyList(blendMultiplyList);

		auto& slots = m_skeletonData.at(i).get()->getSlots();
		for (size_t ii = 0; ii < slots.size(); ++ii)
		{
			std::string strName = slots[ii]->getName().buffer();
			for (const std::string& str : blendScreenList)
			{
				if (strstr(strName.c_str(), str.c_str()))
				{
					slots[ii]->setBlendMode(spine::BlendMode::BlendMode_Screen);
				}
			}
		}

		auto& animations = m_skeletonData.at(i).get()->getAnimations();
		for (size_t ii = 0; ii < animations.size(); ++ii)
		{
			std::string strAnimationName = animations[ii]->getName().buffer();
			auto iter = std::find(m_animationNames.begin(), m_animationNames.end(), strAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(strAnimationName);
		}
	}

	/*寝室再生順*/
	const std::vector<std::string> fixedNames = { "Wait", "Normal", "Fast", "Finish", "After" };
	auto IsR18 = [&fixedNames](const std::string& str)
		-> bool
		{
			for (const std::string& fixedName : fixedNames)
			{
				if(strstr(str.c_str(), fixedName.c_str()) != nullptr)return true;
			}
			return false;
		};
	if (std::all_of(m_animationNames.begin(), m_animationNames.end(), IsR18))
	{
		m_animationNames = fixedNames;
	}

	if (!m_animationNames.empty())
	{
		for (size_t i = 0; i < m_skeletonData.size(); ++i)
		{
			spine::SkeletonDrawable* drawable = m_drawables.back().get();
			drawable->state->setAnimation(0, m_animationNames.at(0).c_str(), true);
		}
	}

	return m_animationNames.size() > 0;
}
/*標準長算出*/
void CSfmlSpinePlayer::WorkOutDefaultSize()
{
	if (m_skeletonData.empty())return;

	unsigned int uiSkeletonWidth = static_cast<unsigned int>(m_skeletonData.at(0).get()->getWidth());
	unsigned int uiSkeletonHeight = static_cast<unsigned int>(m_skeletonData.at(0).get()->getHeight());

	unsigned int uiDesktopWidth = sf::VideoMode::getDesktopMode().width;
	unsigned int uiDesktopHeight = sf::VideoMode::getDesktopMode().height;

	if (uiSkeletonWidth < uiDesktopWidth && uiSkeletonHeight < uiDesktopHeight)
	{
		m_fDefaultScale = 1.f;
	}
	else
	{
		if (uiDesktopWidth > uiDesktopHeight)
		{
			m_fDefaultScale = static_cast<float>(uiDesktopWidth) / uiSkeletonWidth;
			m_iDefaultOffset = sf::Vector2i(0, (uiSkeletonHeight - uiDesktopHeight) / 2);
		}
		else
		{
			m_fDefaultScale = static_cast<float>(uiDesktopHeight) / uiSkeletonHeight;
			m_iDefaultOffset = sf::Vector2i((uiSkeletonWidth - uiDesktopWidth) / 2, 0);
		}

		m_fSkeletonScale = m_fDefaultScale;
	}
}
/*尺度変更適用*/
void CSfmlSpinePlayer::RescaleSkeleton()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->skeleton->setScaleX(m_fSkeletonScale > 0.99f ? m_fSkeletonScale : 1.f);
		m_drawables.at(i).get()->skeleton->setScaleY(m_fSkeletonScale > 0.99f ? m_fSkeletonScale : 1.f);
	}

	unsigned int uiWindowWidthMax = static_cast<unsigned int>(m_fMaxWidth * (m_fSkeletonScale - 0.025f));
	unsigned int uiWindowHeightMax = static_cast<unsigned int>(m_fMaxHeight * (m_fSkeletonScale - 0.025f));
	if (uiWindowWidthMax < sf::VideoMode::getDesktopMode().width || uiWindowHeightMax < sf::VideoMode::getDesktopMode().height)
	{
		ResizeWindow();
	}
}
/*速度変更適用*/
void CSfmlSpinePlayer::RescaleTime()
{
	if (m_fTimeScale < 0.f)m_fTimeScale = 0.f;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->timeScale = m_fTimeScale;
	}
}
/*速度・尺度・視点初期化*/
void CSfmlSpinePlayer::ResetScale()
{
	m_fTimeScale = 1.0f;
	m_fSkeletonScale = m_fDefaultScale;
	m_iOffset = m_iDefaultOffset;

	RescaleSkeleton();
	RescaleTime();
	MoveViewPoint(0, 0);
	ResizeWindow();
}
/*窓寸法調整*/
void CSfmlSpinePlayer::ResizeWindow()
{
	if (m_window.get() != nullptr)
	{
		m_window->setSize(sf::Vector2u(static_cast<unsigned int>(m_fMaxWidth * m_fSkeletonScale), static_cast<unsigned int>(m_fMaxHeight * m_fSkeletonScale)));
	}
}
/*視点移動*/
void CSfmlSpinePlayer::MoveViewPoint(int iX, int iY)
{
	m_iOffset.x += iX;
	m_iOffset.y += iY;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->skeleton->setPosition((m_fMaxWidth - m_iOffset.x) / 2, (m_fMaxHeight - m_iOffset.y) / 2);
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
/*再描画*/
void CSfmlSpinePlayer::Redraw(float fDelta)
{
	if (m_window.get() != nullptr)
	{
		m_window->clear();
		for (size_t i = 0; i < m_drawables.size(); ++i)
		{
			m_drawables.at(i).get()->update(fDelta);
			m_window->draw(*m_drawables.at(i).get(), sf::RenderStates(sf::BlendAlpha));
		}
		m_window->display();
	}
}
