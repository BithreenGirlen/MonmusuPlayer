

#include "sfml_main_window.h"

CSfmlMainWindow::CSfmlMainWindow(const wchar_t* swzWindowName)
{
	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(200, 200), swzWindowName, sf::Style::None);

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

	m_animationIndex = 0;
	ShiftAnimation();

	UpdateMessageText();

	sf::Vector2i iMouseStartPos;

	bool bOnWindowMove = false;
	bool bLeftDowned = false;
	bool bLeftCombinated = false;

	m_spineClock.restart();
	while (m_window->isOpen())
	{
		sf::Event event;
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

					bLeftDowned = true;
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					if (bLeftCombinated)
					{
						bLeftCombinated = false;
						bLeftDowned = false;
						break;
					}

					if (bOnWindowMove || sf::Mouse::isButtonPressed(sf::Mouse::Right))
					{
						bOnWindowMove ^= true;
						break;
					}

					int iX = iMouseStartPos.x - event.mouseButton.x;
					int iY = iMouseStartPos.y - event.mouseButton.y;

					if (iX == 0 && iY == 0 && m_animationNames.empty())
					{
						ShiftAnimation();
					}

					bLeftDowned = false;
				}
				if (event.mouseButton.button == sf::Mouse::Middle)
				{
					m_sfmlSpinePlayer->ResetScale();
					ResizeWindow();
				}
				break;
			case sf::Event::MouseMoved:
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					if (bLeftDowned)
					{
						int iX = iMouseStartPos.x - event.mouseMove.x;
						int iY = iMouseStartPos.y - event.mouseMove.y;
						m_sfmlSpinePlayer->MoveViewPoint(iX, iY);

						iMouseStartPos.x = event.mouseMove.x;
						iMouseStartPos.y = event.mouseMove.y;

						bLeftCombinated = true;
					}
				}
				break;
			case sf::Event::MouseWheelScrolled:
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					m_sfmlSpinePlayer->RescaleTime(event.mouseWheelScroll.delta < 0);
					bLeftCombinated = true;
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
				{
					ShiftMessageText(event.mouseWheelScroll.delta < 0);
				}
				else
				{
					m_sfmlSpinePlayer->RescaleSkeleton(event.mouseWheelScroll.delta < 0);
					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
					{
						m_sfmlSpinePlayer->RescaleCanvas(event.mouseWheelScroll.delta < 0);
						ResizeWindow();
					}
				}
				break;
			case sf::Event::KeyPressed:
				switch (event.key.code)
				{
				case sf::Keyboard::Key::Left:
					ShiftMessageText(false);
					break;
				case sf::Keyboard::Key::Right:
					if (m_nTextIndex < m_textData.size() - 1)
					{
						ShiftMessageText(true);
					}
					break;
				}
				break;
			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::Key::B:
					m_sfmlSpinePlayer->ToggleBlendModeAdoption();
					break;
				case sf::Keyboard::Key::C:
					ToggleTextColor();
					break;
				case sf::Keyboard::Key::R:
					m_sfmlSpinePlayer->ToggleDrawOrder();
					break;
				case sf::Keyboard::Key::S:
					SaveCurrentFrameImage();
					break;
				case sf::Keyboard::Key::T:
					ToggleTextVisibility();
					break;
				case sf::Keyboard::Key::Escape:
					m_window->close();
					break;
				case sf::Keyboard::Key::Up:
					return 2;
				case sf::Keyboard::Key::Down:
					return 1;
				default:
					break;
				}
				break;
			}
		}

		float fDelta = m_spineClock.getElapsedTime().asSeconds();
		m_sfmlSpinePlayer->Update(fDelta);
		m_spineClock.restart();

		m_window->clear(sf::Color(0, 0, 0, 0));

		m_sfmlSpinePlayer->Redraw();
		if (!m_bTextHidden)
		{
			m_window->draw(m_msgText);
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

	return 0;
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
	bool bRet = texture.create(m_window->getSize().x, m_window->getSize().y);
	if (!bRet)return false;

	texture.update(*m_window);
	sf::Image image = texture.copyToImage();

	bRet = image.saveToFile(strStrFilePath);

	return bRet;
}

void CSfmlMainWindow::ShiftAnimation()
{
	constexpr const char* const animations[] = { "Wait", "Normal", "Fast", "Finish", "After" };
	constexpr size_t animationCount = sizeof(animations) / sizeof(animations[0]);

	if (m_animationIndex < animationCount)
	{
		m_sfmlSpinePlayer->SetAnimationByName(animations[m_animationIndex]);
	}
	if (++m_animationIndex >= animationCount)
	{
		m_animationIndex = 0;
	}
}
/*字体設定*/
bool CSfmlMainWindow::SetFont(const std::string& strFilePath, bool bBold, bool bItalic)
{
	bool bRet = m_font.loadFromFile(strFilePath);
	if (!bRet)return false;

	constexpr float fOutLineThickness = 2.4f;

	m_msgText.setFont(m_font);
	m_msgText.setFillColor(sf::Color::Black);
	m_msgText.setStyle((bBold ? sf::Text::Style::Bold : 0) | (bItalic ? sf::Text::Style::Italic : 0));
	m_msgText.setOutlineThickness(fOutLineThickness);
	m_msgText.setOutlineColor(sf::Color::White);

	return true;
}

void CSfmlMainWindow::SetScenarioData(std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames)
{
	m_textData = std::move(textData);
	m_animationNames = std::move(animationNames);

	m_nTextIndex = 0;
	m_msgText.setString("");
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
}
/*文字色切り替え*/
void CSfmlMainWindow::ToggleTextColor()
{
	m_msgText.setFillColor(m_msgText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
	m_msgText.setOutlineColor(m_msgText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
}

void CSfmlMainWindow::ToggleTextVisibility()
{
	m_bTextHidden ^= true;
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
	if (m_textData.empty())return;

	const adv::TextDatum& textDatum = m_textData.at(m_nTextIndex);
	std::wstring wstr = textDatum.wstrText;

	constexpr unsigned int kLineThreashold = 46;
	for (size_t i = kLineThreashold; i < wstr.size(); i += kLineThreashold)
	{
		wstr.insert(i, L"\n");
	}
	if (!wstr.empty() && wstr.back() != L'\n') wstr += L"\n ";

	wstr += std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
	m_msgText.setString(wstr);

	/* Checks if animation has to be switched or not. */
	//if (m_nLastAnimationIndex != textDatum.nAnimationIndex)
	//{
	//	if (textDatum.nAnimationIndex < m_animationNames.size())
	//	{
	//		m_nLastAnimationIndex = textDatum.nAnimationIndex;
	//		m_sfmlSpinePlayer->SetAnimationByName(m_animationNames[m_nLastAnimationIndex].c_str());
	//	}
	//}

	if (!textDatum.wstrVoicePath.empty())
	{
		if (m_pAudioPlayer.get() != nullptr)
		{
			m_pAudioPlayer->Play(textDatum.wstrVoicePath.c_str());
		}
	}
	m_textClock.restart();
}