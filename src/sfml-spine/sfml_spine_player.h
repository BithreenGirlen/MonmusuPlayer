#ifndef SFML_SPINE_PLAYER_H_
#define SFML_SPINE_PLAYER_H_

#include"spine_player.h"

class CSfmlSpinePlayer : public CSpinePlayer
{
public:
	CSfmlSpinePlayer() = default;
	virtual ~CSfmlSpinePlayer() = default;

	void redraw(sf::RenderTarget* pRenderTarget, sf::RenderStates& renderState);

	sf::Transform calculateTransformMatrix(sf::RenderTarget* pRenderTarget) const noexcept;
	sf::FloatRect getCurrentBoundingBox() const;
	sf::FloatRect getCurrentBoundingBoxOfSlot(std::string_view slotName) const;
private:
	void workOutDefaultScale() override;
	bool workOutDefaultSizeAndOffset() override;
};

#endif // !SFML_SPINE_PLAYER_H_
