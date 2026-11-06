

#include "sfml_spine_player.h"

void CSfmlSpinePlayer::redraw(sf::RenderTarget* pRenderTarget, sf::RenderStates& renderState)
{
	if (pRenderTarget != nullptr && !m_drawables.empty())
	{
		if (!m_isDrawOrderReversed)
		{
			for (size_t i = 0; i < m_drawables.size(); ++i)
			{
				pRenderTarget->draw(*m_drawables[i], renderState);
			}
		}
		else
		{
			for (long long i = m_drawables.size() - 1; i >= 0; --i)
			{
				pRenderTarget->draw(*m_drawables[i], renderState);
			}
		}
	}
}

sf::Transform CSfmlSpinePlayer::calculateTransformMatrix(sf::RenderTarget* pRenderTarget) const noexcept
{
	const sf::Vector2u targetSize = pRenderTarget->getSize();
	const float fX = (m_fBaseSize.x * m_fSkeletonScale - targetSize.x) / 2;
	const float fY = (m_fBaseSize.y * m_fSkeletonScale - targetSize.y) / 2;

	return sf::Transform().translate({ -fX, -fY }).scale({ m_fSkeletonScale, m_fSkeletonScale });
}

sf::FloatRect CSfmlSpinePlayer::getCurrentBoundingBox() const
{
	float fMinX = std::numeric_limits<float>::max();
	float fMinY = std::numeric_limits<float>::max();
	float fMaxX = std::numeric_limits<float>::lowest();
	float fMaxY = std::numeric_limits<float>::lowest();

	for (const auto& drawable : m_drawables)
	{
		const sf::FloatRect rect = drawable->getBoundingBox();
		fMinX = (std::min)(fMinX, rect.position.x);
		fMinY = (std::min)(fMinY, rect.position.y);
		fMaxX = (std::max)(fMaxX, rect.position.x + rect.size.x);
		fMaxY = (std::max)(fMaxY, rect.position.y + rect.size.y);
	}

	return { {fMinX, fMinY}, {fMaxX - fMinX, fMaxY - fMinY} };
}

sf::FloatRect CSfmlSpinePlayer::getCurrentBoundingBoxOfSlot(std::string_view slotName) const
{
	bool found = false;
	for (const auto& drawable : m_drawables)
	{
		const auto& rect = drawable->getBoundingBoxOfSlot(slotName.data(), slotName.size(), &found);
		if (found)
		{
			return rect;
		}
	}

	return {};
}

void CSfmlSpinePlayer::workOutDefaultScale()
{
	m_fDefaultScale = 1.f;

	const unsigned int skeletonWidth = static_cast<unsigned int>(m_fBaseSize.x);
	const unsigned int skeletonHeight = static_cast<unsigned int>(m_fBaseSize.y);

	/* Should refer to the monitor where the window is, not the priomary. */
	const unsigned int displayWidth = sf::VideoMode::getDesktopMode().size.x;
	const unsigned int displayHeight = sf::VideoMode::getDesktopMode().size.y;

	if (skeletonWidth > displayWidth || skeletonHeight > displayHeight)
	{
		const float fScaleX = static_cast<float>(displayWidth) / skeletonWidth;
		const float fScaleY = static_cast<float>(displayHeight) / skeletonHeight;

		m_fDefaultScale = fScaleX > fScaleY ? fScaleY : fScaleX;
	}
}

bool CSfmlSpinePlayer::workOutDefaultSizeAndOffset()
{
	static constexpr float kMaxDimension = 16384.f;

	float fMinX = std::numeric_limits<float>::max();
	float fMinY = std::numeric_limits<float>::max();
	float fWidth = std::numeric_limits<float>::lowest();
	float fHeight = std::numeric_limits<float>::lowest();

	for (const auto& pDrawable : m_drawables)
	{
		const sf::FloatRect rect = pDrawable->getBoundingBox();
		fMinX = (std::min)(fMinX, rect.position.x);
		fMinY = (std::min)(fMinY, rect.position.y);

		if (::isless(rect.size.x, kMaxDimension) && ::isless(rect.size.y, kMaxDimension))
		{
			fWidth = (std::max)(fWidth, rect.size.x);
			fHeight = (std::max)(fHeight, rect.size.y);
		}
	}

	if (::isless(fMinX, std::numeric_limits<float>::max()) && ::isless(fMinY, std::numeric_limits<float>::max()))
	{
		m_fDefaultOffset.x = fMinX;
		m_fDefaultOffset.y = fMinY;
	}

	if (::isgreater(fWidth, std::numeric_limits<float>::lowest()) && ::isgreater(fHeight, std::numeric_limits<float>::lowest()))
	{
		m_fBaseSize.x = fWidth;
		m_fBaseSize.y = fHeight;

		return true;
	}

	return false;
}
