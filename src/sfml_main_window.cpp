

#include "sfml_main_window.h"

CSfmlMainWindow::CSfmlMainWindow(const wchar_t* swzWindowName)
{
	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode({ 200, 200 }), swzWindowName, sf::Style::None);

	m_window->setPosition(sf::Vector2i(0, 0));

	m_sfmlSpinePlayer = std::make_unique<CSfmlSpinePlayer>(m_window.get());
}

CSfmlMainWindow::~CSfmlMainWindow()
{

}

bool CSfmlMainWindow::SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	return m_sfmlSpinePlayer->LoadSpineFromFile(atlasPaths, skelPaths, isBinarySkel);
}

void CSfmlMainWindow::SetSlotsToExclude(const std::vector<std::string>& slotNames)
{
	m_sfmlSpinePlayer->SetSlotsToExclude(slotNames);
}
void CSfmlMainWindow::SetSlotExclusionCallback(bool (*pFunc)(const char*, size_t))
{
	m_sfmlSpinePlayer->SetSlotExclusionCallback(pFunc);
}

int CSfmlMainWindow::Display()
{
	m_sfmlSpinePlayer->ResetScale();
	ResizeWindow();

	UpdateMessageText();

	sf::Vector2i iMouseStartPos;

	bool bOnWindowMove = false;
	bool bLeftDowned = false;
	bool bLeftCombinated = false;

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
					iMouseStartPos = event.position;

					bLeftDowned = true;
				}
			},
			[&](const sf::Event::MouseButtonReleased& event)
			{
				if (event.button == sf::Mouse::Button::Left)
				{
					if (bLeftCombinated)
					{
						bLeftCombinated = false;
						bLeftDowned = false;
						return;
					}

					if (bOnWindowMove || sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
					{
						bOnWindowMove ^= true;
						return;
					}

					int iX = iMouseStartPos.x - event.position.x;
					int iY = iMouseStartPos.y - event.position.y;

					if (iX == 0 && iY == 0)
					{
						ShiftAnimation();
					}

					bLeftDowned = false;
				}
				else if (event.button == sf::Mouse::Button::Middle)
				{
					m_sfmlSpinePlayer->ResetScale();
					ResizeWindow();
				}
			},
			[&](const sf::Event::MouseMoved& event)
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (bLeftDowned)
					{
						int iX = iMouseStartPos.x - event.position.x;
						int iY = iMouseStartPos.y - event.position.y;
						m_sfmlSpinePlayer->MoveViewPoint(iX, iY);

						iMouseStartPos = event.position;

						bLeftCombinated = true;
					}
				}
			},
			[&](const sf::Event::MouseWheelScrolled& event)
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					m_sfmlSpinePlayer->RescaleTime(event.delta < 0);
					bLeftCombinated = true;
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
				{
					ShiftMessageText(event.delta < 0);
				}
				else
				{
					m_sfmlSpinePlayer->RescaleSkeleton(event.delta < 0);
					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
					{
						m_sfmlSpinePlayer->RescaleCanvas(event.delta < 0);
						ResizeWindow();
					}
				}
			},
			[&](const sf::Event::KeyPressed& event)
			{
				switch (event.scancode)
				{
				case sf::Keyboard::Scancode::Left:
					ShiftMessageText(false);
					break;
				case sf::Keyboard::Scancode::Right:
					if (m_nTextIndex < m_textData.size() - 1)
					{
						ShiftMessageText(true);
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
				case sf::Keyboard::Scancode::B:
					m_sfmlSpinePlayer->ToggleBlendModeAdoption();
					break;
				case sf::Keyboard::Scancode::C:
					ToggleTextColor();
					break;
				case sf::Keyboard::Scancode::R:
					m_sfmlSpinePlayer->ToggleDrawOrder();
					break;
				case sf::Keyboard::Scancode::S:
					SaveCurrentFrameImage();
					break;
				case sf::Keyboard::Scancode::T:
					ToggleTextVisibility();
					break;
				case sf::Keyboard::Scancode::Q:
					ToggleImageSync();
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

		float fDelta = m_spineClock.getElapsedTime().asSeconds();
		m_sfmlSpinePlayer->Update(fDelta);
		m_spineClock.restart();

		m_window->clear(sf::Color(0, 0, 0, 0));

		m_sfmlSpinePlayer->Redraw();
		if (!m_bTextHidden)
		{
			m_window->draw(*m_msgText);
		}

		m_window->display();

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

void CSfmlMainWindow::ResizeWindow()
{
	if (m_sfmlSpinePlayer.get() != nullptr)
	{
		sf::Vector2f fBaseSize = m_sfmlSpinePlayer->GetBaseSize();
		float fScale = m_sfmlSpinePlayer->GetCanvasScale();

		unsigned int uiWindowWidthMax = static_cast<unsigned int>(fBaseSize.x * fScale);
		unsigned int uiWindowHeightMax = static_cast<unsigned int>(fBaseSize.y * fScale);

		m_window->setSize(sf::Vector2u(static_cast<unsigned int>(fBaseSize.x * fScale), static_cast<unsigned int>(fBaseSize.y * fScale)));
		m_window->setView(sf::View((fBaseSize * fScale) / 2.f, fBaseSize * fScale));
	}
}

bool CSfmlMainWindow::SaveCurrentFrameImage()
{
	/*SFML does not have API equivalent to SDL's SDL_GetBasePath().*/
	float fTrackTime = 0.f;
	m_sfmlSpinePlayer->GetCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);
	const char* pzAnimationName = m_sfmlSpinePlayer->GetCurrentAnimationName();
	if (pzAnimationName == nullptr)return false;

	std::string strStrFilePath = pzAnimationName;
	strStrFilePath += "_" + std::to_string(fTrackTime) + ".png";

	sf::Texture texture;
	bool bRet = texture.resize({ m_window->getSize().x, m_window->getSize().y });
	if (!bRet)return false;

	texture.update(*m_window);
	sf::Image image = texture.copyToImage();

	bRet = image.saveToFile(strStrFilePath);

	return bRet;
}
/*字体設定*/
bool CSfmlMainWindow::SetFont(const std::string& strFilePath, bool bBold, bool bItalic)
{
	bool bRet = m_font.openFromFile(strFilePath);
	if (!bRet)return false;

	constexpr float fOutLineThickness = 2.4f;
	m_msgText = std::make_unique<sf::Text>(m_font);
	m_msgText->setFillColor(sf::Color::Black);
	m_msgText->setStyle((bBold ? sf::Text::Style::Bold : 0) | (bItalic ? sf::Text::Style::Italic : 0));
	m_msgText->setOutlineThickness(fOutLineThickness);
	m_msgText->setOutlineColor(sf::Color::White);

	return true;
}

void CSfmlMainWindow::SetScenarioData(std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames)
{
	m_textData = std::move(textData);
	m_animationNames = std::move(animationNames);

	m_nTextIndex = 0;
	m_msgText->setString("");
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
		m_sfmlSpinePlayer->SetAnimationByName(m_animationNames[0].c_str());
	}
}
/*文字色切り替え*/
void CSfmlMainWindow::ToggleTextColor()
{
	m_msgText->setFillColor(m_msgText->getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
	m_msgText->setOutlineColor(m_msgText->getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
}

void CSfmlMainWindow::ToggleTextVisibility()
{
	m_bTextHidden ^= true;
}

void CSfmlMainWindow::ToggleImageSync()
{
	m_bImageSynced ^= true;
	if (m_bImageSynced)
	{
		CheckAnimationTrack();
	}
}
/*文章切り替え是否確認*/
void CSfmlMainWindow::CheckTimer()
{
	constexpr float fAutoPlayInterval = 2.f;
	float fSecond = m_textClock.getElapsedTime().asSeconds();
	if (m_pAudioPlayer.get() != nullptr && m_pAudioPlayer.get()->IsEnded() && fSecond > fAutoPlayInterval)
	{
		if (m_nTextIndex < m_textData.size() - 1)
		{
			ShiftMessageText(true);
		}
		else
		{
			m_textClock.restart();
		}
	}
}
/*表示文章移行*/
void CSfmlMainWindow::ShiftMessageText(bool bForward)
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
/*表示文章変更適用*/
void CSfmlMainWindow::UpdateMessageText()
{
	if (m_nTextIndex >= m_textData.size())return;

	const adv::TextDatum& textDatum = m_textData[m_nTextIndex];
	std::wstring wstr = textDatum.wstrText;
	if (!wstr.empty() && wstr.back() != L'\n') wstr += L"\n ";

	wstr += std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
	m_msgText->setString(wstr);

	/* Checks if animation has to be switched or not. */
	if (m_bImageSynced)
	{
		CheckAnimationTrack();
	}

	if (!textDatum.wstrVoicePath.empty())
	{
		if (m_pAudioPlayer.get() != nullptr)
		{
			m_pAudioPlayer->Play(textDatum.wstrVoicePath.c_str());
		}
	}
	m_textClock.restart();
}
void CSfmlMainWindow::ShiftAnimation()
{
	if (m_animationNames.empty())
	{
		m_sfmlSpinePlayer->ShiftAnimation();
	}
	else if (!m_bImageSynced)
	{
		if (++m_nLastAnimationIndex >= m_animationNames.size())
		{
			m_nLastAnimationIndex = 0;
		}

		m_sfmlSpinePlayer->SetAnimationByName(m_animationNames[m_nLastAnimationIndex].c_str());
	}
}

void CSfmlMainWindow::CheckAnimationTrack()
{
	if (m_nTextIndex >= m_textData.size())return;

	const adv::TextDatum& textDatum = m_textData[m_nTextIndex];
	if (m_nLastAnimationIndex != textDatum.nAnimationIndex)
	{
		if (textDatum.nAnimationIndex < m_animationNames.size())
		{
			m_nLastAnimationIndex = textDatum.nAnimationIndex;
			m_sfmlSpinePlayer->SetAnimationByName(m_animationNames[m_nLastAnimationIndex].c_str());
		}
	}
}