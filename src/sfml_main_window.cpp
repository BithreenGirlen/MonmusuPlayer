

#include "sfml_main_window.h"

CSfmlMainWindow::CSfmlMainWindow(const char32_t* windowName)
{
	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode({ 200, 200 }), windowName, sf::Style::None);

	alignWindowToTheTopLeftOfMonitor();

	m_sfmlSpinePlayer = std::make_unique<CSfmlSpinePlayer>();
}

bool CSfmlMainWindow::setSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	return m_sfmlSpinePlayer->loadSpineFromFile(atlasPaths, skelPaths, isBinarySkel);
}

void CSfmlMainWindow::setSlotExclusionCallback(bool (*pFunc)(const char*, size_t)) noexcept
{
	m_sfmlSpinePlayer->setSlotExcludeCallback(pFunc);
}

int CSfmlMainWindow::display()
{
	m_sfmlSpinePlayer->resetScale();
	resizeWindow();

	updateMessageText();

	int iRet = 0;

	m_spineClock.restart();
	while (m_window->isOpen())
	{
		m_window->handleEvents
		(
			[&](const sf::Event::Closed&)
			{
				m_window->close();
			},
			[&](const sf::Event::MouseButtonPressed& event)
			{
				if (event.button == sf::Mouse::Button::Left)
				{
					m_mouseState.lastMousePos = event.position;

					m_mouseState.wasLeftPressed = true;
				}
			},
			[&](const sf::Event::MouseButtonReleased& event)
			{
				if (event.button == sf::Mouse::Button::Left)
				{
					if (m_mouseState.wasLeftCombined || m_mouseState.wasLeftDragged)
					{
						m_mouseState.wasLeftCombined = false;
						m_mouseState.wasLeftPressed = false;
						m_mouseState.wasLeftDragged = false;

						return;
					}

					if (m_windowState.toBeMoved || sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
					{
						m_windowState.toBeMoved ^= true;
						return;
					}

					const sf::Vector2i mouseDelta = m_mouseState.lastMousePos - event.position;
					if (mouseDelta.x == 0 && mouseDelta.y == 0)
					{
						shiftAnimation();
					}

					m_mouseState.wasLeftPressed = false;
				}
				else if (event.button == sf::Mouse::Button::Middle)
				{
					m_sfmlSpinePlayer->resetScale();
					resizeWindow();
				}
			},
			[&](const sf::Event::MouseMoved& event)
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (!m_mouseState.wasLeftCombined)
					{
						if (m_mouseState.wasLeftDragged)
						{
							const sf::Vector2i mouseDelta = m_mouseState.lastMousePos - event.position;
							m_sfmlSpinePlayer->addOffset(mouseDelta.x, mouseDelta.y);
						}

						m_mouseState.lastMousePos = event.position;

						m_mouseState.wasLeftDragged = true;
					}
				}
			},
			[&](const sf::Event::MouseWheelScrolled& event)
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					static constexpr float kTimeScaleDelta = 0.05f;
					const float scrollSign = event.delta < 0 ? 1.f : -1.f;

					float timeScale = m_sfmlSpinePlayer->getTimeScale() + kTimeScaleDelta * scrollSign;
					timeScale = (std::max)(timeScale, 0.f);
					m_sfmlSpinePlayer->setTimeScale(timeScale);

					m_mouseState.wasLeftCombined = true;
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
				{
					shiftMessageText(event.delta < 0);
				}
				else
				{
					static constexpr float kMinScale = 0.15f;
					const float scrollSign = event.delta < 0 ? 1.f : -1.f;

					float skeletonScale = m_sfmlSpinePlayer->getSkeletonScale() + kScaleDelta * scrollSign;
					skeletonScale = (std::max)(kMinScale, skeletonScale);
					m_sfmlSpinePlayer->setSkeletonScale(skeletonScale);

					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
					{
						float canvasScale = m_sfmlSpinePlayer->getCanvasScale() + kScaleDelta * scrollSign;
						canvasScale = (std::max)(kMinScale, canvasScale);
						m_sfmlSpinePlayer->setCanvasScale(canvasScale);

						resizeWindow();
					}
				}
			},
			[&](const sf::Event::KeyPressed& event)
			{
				switch (event.scancode)
				{
				case sf::Keyboard::Scancode::Left:
					shiftMessageText(false);
					break;
				case sf::Keyboard::Scancode::Right:
					if (m_nTextIndex < m_textData.size() - 1)
					{
						shiftMessageText(true);
					}
					break;
				default:
					break;
				}
			},
			[&](const sf::Event::KeyReleased& event)
			{
				switch (event.scancode)
				{
				case sf::Keyboard::Scancode::C:
					toggleTextColor();
					break;
				case sf::Keyboard::Scancode::S:
					saveCurrentFrameToFile();
					break;
				case sf::Keyboard::Scancode::T:
					toggleTextVisibility();
					break;
				case sf::Keyboard::Scancode::Q:
					toggleImageSync();
					break;
				case sf::Keyboard::Scancode::Escape:
					m_window->close();
					break;
				case sf::Keyboard::Scancode::Up:
					iRet = 2;
					break;
				case sf::Keyboard::Scancode::Down:
					iRet = 1;
					break;
				default:
					break;
				}
			}
		);

		if (iRet != 0)break;

		const float fDelta = m_spineClock.getElapsedTime().asSeconds();
		m_sfmlSpinePlayer->update(fDelta);
		m_spineClock.restart();

		m_window->clear(sf::Color(0, 0, 0, 0));
		{
			sf::RenderStates spineRenderStates(m_sfmlSpinePlayer->calculateTransformMatrix(&m_spineRenderTexture));

			m_spineRenderTexture.clear();
			m_sfmlSpinePlayer->redraw(&m_spineRenderTexture, spineRenderStates);
			m_spineRenderTexture.display();

			sf::Sprite spineSprite(m_spineRenderTexture.getTexture());
			m_window->draw(spineSprite);
		}

		if (!m_sceneState.isTextHidden)
		{
			m_window->draw(*m_msgText);
		}

		m_window->display();

		checkTimer();

		if (m_windowState.toBeMoved)
		{
			const int iWindowPosX = sf::Mouse::getPosition().x - m_window->getSize().x / 2;
			const int iWindowPosY = sf::Mouse::getPosition().y - m_window->getSize().y / 2;
			m_window->setPosition(sf::Vector2i(iWindowPosX, iWindowPosY));
		}
	}

	return iRet;
}

void CSfmlMainWindow::resizeWindow()
{
	sf::Vector2f fBaseSize = m_sfmlSpinePlayer->getBaseSize();
	float fScale = m_sfmlSpinePlayer->getCanvasScale();

	const unsigned int maxWindowWidth = static_cast<unsigned int>(fBaseSize.x * (fScale - kScaleDelta));
	const unsigned int maxWindowHeight = static_cast<unsigned int>(fBaseSize.y * (fScale - kScaleDelta));

	const sf::Vector2u monitorSize = getMonitorInfo().size;
	/*
	* Instability of borderless window with max resolution has not been fixed even in SFML 3.0.2
	* https://github.com/SFML/SFML/issues/1284
	*/
	if (maxWindowWidth < monitorSize.x || maxWindowHeight < monitorSize.y)
	{
		const unsigned int windowWidth = static_cast<unsigned int>(fBaseSize.x * fScale);
		const unsigned int windowHeight = static_cast<unsigned int>(fBaseSize.y * fScale);

		m_window->setSize(sf::Vector2u(windowWidth, windowHeight));
		m_window->setView(sf::View((fBaseSize * fScale) / 2.f, fBaseSize * fScale));

		bool bRet = m_spineRenderTexture.resize({ windowWidth, windowHeight });
	}
}

void CSfmlMainWindow::alignWindowToTheTopLeftOfMonitor()
{
	m_window->setPosition(getMonitorInfo().position);
}

CSfmlMainWindow::MonitorInfo CSfmlMainWindow::getMonitorInfo()
{
	/* SFML does know only primary monitor. */
#if defined (_WIN32)
	MONITORINFO monitorInfo{ sizeof(MONITORINFO) };
	HMONITOR hMonitor = ::MonitorFromWindow(m_window->getNativeHandle(), MONITOR_DEFAULTTONEAREST);
	if (hMonitor != nullptr)
	{
		::GetMonitorInfoW(hMonitor, &monitorInfo);
	}

	const int x = static_cast<int>(monitorInfo.rcMonitor.left);
	const int y = static_cast<int>(monitorInfo.rcMonitor.top);

	const unsigned int width = static_cast<unsigned int>(monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
	const unsigned int height = static_cast<unsigned int>(monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);

	return { {x, y}, {width, height} };
#else
	return { {}, sf::VideoMode::getDesktopMode().size };
#endif
}

bool CSfmlMainWindow::saveCurrentFrameToFile()
{
	/*SFML does not have API equivalent to SDL's SDL_GetBasePath().*/
	float fTrackTime = 0.f;
	m_sfmlSpinePlayer->getCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);
	const char* pzAnimationName = m_sfmlSpinePlayer->getCurrentAnimationName();
	if (pzAnimationName == nullptr)return false;

	std::string strFilePath = pzAnimationName;
	char sBuffer[16]{};
	sprintf_s(sBuffer, "_%.3f.png", fTrackTime);
	strFilePath += sBuffer;

	sf::Texture texture = m_spineRenderTexture.getTexture();
	sf::Image image = texture.copyToImage();

	return image.saveToFile(strFilePath);
}
/*字体設定*/
bool CSfmlMainWindow::setFont(const std::string& fontFilePath, bool bold, bool italic)
{
	bool bRet = m_font.openFromFile(fontFilePath);
	if (!bRet)return false;

	static constexpr float kOutLineThickness = 2.4f;

	static const auto GetDpiScale = [](const sf::Window* const pWindow)
		-> float
		{
#if defined (_WIN32)
			return ::GetDpiForWindow(pWindow->getNativeHandle()) / 96.f;
#else
			return 1.f
#endif
		};
	const float dpiScale = GetDpiScale(m_window.get());

	m_msgText = std::make_unique<sf::Text>(m_font);
	m_msgText->setFillColor(sf::Color::Black);
	m_msgText->setStyle((bold ? sf::Text::Style::Bold : 0) | (italic ? sf::Text::Style::Italic : 0));
	m_msgText->setCharacterSize(static_cast<unsigned int>(m_msgText->getCharacterSize() * dpiScale));
	m_msgText->setOutlineThickness(kOutLineThickness * dpiScale);
	m_msgText->setOutlineColor(sf::Color::White);

	return true;
}

void CSfmlMainWindow::setScenarioData(std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames)
{
	m_textData = std::move(textData);
	m_animationNames = std::move(animationNames);

	m_nTextIndex = 0;
	m_msgText->setString(U"");
	m_nLastAnimationIndex = 0;

	const auto HasAudio = [](const adv::TextDatum& text)
		-> bool
		{
			return !text.wstrVoicePath.empty();
		};
	const auto iter = std::find_if(m_textData.begin(), m_textData.end(), HasAudio);
	if (iter != m_textData.cend())
	{
		m_pAudioPlayer = std::make_unique<CMfMediaPlayer>();
	}

	if (!m_animationNames.empty())
	{
		m_sfmlSpinePlayer->setAnimationByName(m_animationNames[0].c_str());
	}
}
/*文字色切り替え*/
void CSfmlMainWindow::toggleTextColor()
{
	m_msgText->setFillColor(m_msgText->getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
	m_msgText->setOutlineColor(m_msgText->getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
}

void CSfmlMainWindow::toggleTextVisibility()
{
	m_sceneState.isTextHidden ^= true;
}

void CSfmlMainWindow::toggleImageSync()
{
	m_sceneState.isImageSynced ^= true;
	if (m_sceneState.isImageSynced)
	{
		checkAnimationTrack();
	}
}
/*文章切り替え是否確認*/
void CSfmlMainWindow::checkTimer()
{
	constexpr float fAutoPlayInterval = 2.f;
	float fSecond = m_textClock.getElapsedTime().asSeconds();
	if (m_pAudioPlayer.get() != nullptr && m_pAudioPlayer.get()->isEnded() && fSecond > fAutoPlayInterval)
	{
		if (m_nTextIndex < m_textData.size() - 1)
		{
			shiftMessageText(true);
		}
		else
		{
			m_textClock.restart();
		}
	}
}
/*表示文章移行*/
void CSfmlMainWindow::shiftMessageText(bool forward)
{
	if (forward)
	{
		++m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
	}
	else
	{
		--m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
	}
	updateMessageText();
}
/*表示文章変更適用*/
void CSfmlMainWindow::updateMessageText()
{
	if (m_nTextIndex >= m_textData.size())return;

	const adv::TextDatum& textDatum = m_textData[m_nTextIndex];
	std::wstring wstr = textDatum.wstrText;
	if (!wstr.empty() && wstr.back() != L'\n') wstr += L"\n ";

	wstr += std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
	m_msgText->setString(wstr);

	/* Checks if animation has to be switched or not. */
	if (m_sceneState.isImageSynced)
	{
		checkAnimationTrack();
	}

	if (!textDatum.wstrVoicePath.empty())
	{
		if (m_pAudioPlayer.get() != nullptr)
		{
			m_pAudioPlayer->play(textDatum.wstrVoicePath.c_str());
		}
	}
	m_textClock.restart();
}
void CSfmlMainWindow::shiftAnimation()
{
	if (m_animationNames.empty())
	{
		m_sfmlSpinePlayer->shiftAnimation();
	}
	else if (!m_sceneState.isImageSynced)
	{
		if (++m_nLastAnimationIndex >= m_animationNames.size())
		{
			m_nLastAnimationIndex = 0;
		}

		m_sfmlSpinePlayer->setAnimationByName(m_animationNames[m_nLastAnimationIndex].c_str());
	}
}

void CSfmlMainWindow::checkAnimationTrack()
{
	if (m_nTextIndex >= m_textData.size())return;

	const adv::TextDatum& textDatum = m_textData[m_nTextIndex];
	if (m_nLastAnimationIndex != textDatum.nAnimationIndex)
	{
		if (textDatum.nAnimationIndex < m_animationNames.size())
		{
			m_nLastAnimationIndex = textDatum.nAnimationIndex;
			m_sfmlSpinePlayer->setAnimationByName(m_animationNames[m_nLastAnimationIndex].c_str());
		}
	}
}