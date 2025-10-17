

#include "sfml_spine_player.h"

CSfmlSpinePlayer::CSfmlSpinePlayer(sf::RenderWindow* pSfmlWindow)
	:m_pSfmlWindow(pSfmlWindow)
{

}

CSfmlSpinePlayer::~CSfmlSpinePlayer()
{

}


void CSfmlSpinePlayer::Redraw()
{
	if (m_pSfmlWindow != nullptr)
	{
		const auto& windowSize = m_pSfmlWindow->getSize();
		float fX = (m_fBaseSize.x * m_fSkeletonScale - windowSize.x) / 2;
		float fY = (m_fBaseSize.y * m_fSkeletonScale - windowSize.y) / 2;

		sf::RenderStates renderState;
		renderState.blendMode = sf::BlendAlpha;
		renderState.transform.translate(-fX, -fY).scale(m_fSkeletonScale, m_fSkeletonScale);

		if (!m_bDrawOrderReversed)
		{
			for (size_t i = 0; i < m_drawables.size(); ++i)
			{
				m_pSfmlWindow->draw(*m_drawables[i], renderState);
			}
		}
		else
		{
			for(long long i = m_drawables.size() - 1;i >= 0;--i)
			{
				m_pSfmlWindow->draw(*m_drawables[i], renderState);
			}
		}
	}
}
/*標準尺度算出*/
void CSfmlSpinePlayer::WorkOutDefaultScale()
{
	m_fDefaultScale = 1.f;

	unsigned int uiSkeletonWidth = static_cast<unsigned int>(m_fBaseSize.x);
	unsigned int uiSkeletonHeight = static_cast<unsigned int>(m_fBaseSize.y);

	unsigned int uiDesktopWidth = sf::VideoMode::getDesktopMode().width;
	unsigned int uiDesktopHeight = sf::VideoMode::getDesktopMode().height;

	if (uiSkeletonWidth > uiDesktopWidth || uiSkeletonHeight > uiDesktopHeight)
	{
		float fScaleX = static_cast<float>(uiDesktopWidth) / uiSkeletonWidth;
		float fScaleY = static_cast<float>(uiDesktopHeight) / uiSkeletonHeight;

		if (uiDesktopWidth > uiDesktopHeight)
		{
			m_fDefaultScale = static_cast<float>(uiDesktopHeight) / uiSkeletonHeight;
		}
		else
		{
			m_fDefaultScale = static_cast<float>(uiDesktopWidth) / uiSkeletonWidth;
		}
		m_fSkeletonScale = m_fDefaultScale;
	}
}

void CSfmlSpinePlayer::WorkOutDefaultOffset()
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;

	for (const auto& pDrawable : m_drawables)
	{
		const auto& rect = pDrawable->GetBoundingBox();
		fMinX = (std::min)(fMinX, rect.left);
		fMinY = (std::min)(fMinY, rect.top);
	}

	m_fDefaultOffset = { fMinX, fMinY };
}
