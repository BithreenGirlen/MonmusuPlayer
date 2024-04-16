#ifndef SFML_SPINE_PLAYER_H_
#define SFML_SPINE_PLAYER_H_

#include <memory>

#include "adv.h"
#include "spine-sfml.h"

/*Windows OS*/
#include "mf_media_player.h"

class CSfmlSpinePlayer
{
public:
	CSfmlSpinePlayer();
	~CSfmlSpinePlayer();
	bool SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary);
	bool SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary);
	bool SetFont(const std::string& strFilePath, bool bBold, bool bItalic);
	void SetTexts(const std::vector<adv::TextDatum>& textData);
	int Display(const wchar_t* pwzWindowName);
private:
	enum Size { kBaseWidth = 1280, kBaseHeight = 700 };

	CSfmlTextureLoader m_textureLoader;
	std::vector<std::unique_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;
	std::vector<std::shared_ptr<CSfmlSpineDrawable>> m_drawables;

	std::unique_ptr<sf::RenderWindow> m_window;
	sf::Vector2f m_BaseWindowSize = sf::Vector2f{ Size::kBaseWidth, Size::kBaseHeight };

	const float m_kfScalePortion = 0.025f;

	float m_fDefaultWindowScale = 1.f;
	float m_fThresholdScale = 1.f;

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	sf::Vector2i m_iOffset{};

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	sf::Font m_font;
	sf::Text m_msgText;
	sf::Clock m_clock;

	size_t m_nTextIndex = 0;
	std::vector<adv::TextDatum> m_textData;
	bool m_bTextHidden = false;

	bool SetupDrawer();
	void WorkOutDefaultScale();

	void RescaleSkeleton(bool bUpscale);
	void RescaleTime(bool bHasten);
	void UpdateScaletonScale();
	void UpdateTimeScale();
	void ResetScale();
	void ResizeWindow();

	void MoveViewPoint(int iX, int iY);
	void ShiftScene();

	void ShiftSkin(bool bForward);
	void UpdateSkin();

	void SwitchTextColor();

	void AdjustTextPosition();

	void CheckTimer();
	void ShiftMessageText(bool bForward);
	void UpdateMessageText();

	void Redraw(float fDelta);

	std::unique_ptr<CMfMediaPlayer> m_pAudioPlayer;
	void ChangePlaybackRate(bool bFaster);
	void ResetPlacybackRate();
};

#endif // SFML_SPINE_PLAYER_H_
