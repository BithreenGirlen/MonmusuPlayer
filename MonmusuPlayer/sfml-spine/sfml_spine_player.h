#ifndef SFML_SPINE_PLAYER_H_
#define SFML_SPINE_PLAYER_H_

#include"spine_player.h"

class CSfmlSpinePlayer : public CSpinePlayer
{
public:
	CSfmlSpinePlayer(sf::RenderWindow* pSfmlWindow);
	virtual ~CSfmlSpinePlayer();

	virtual void Redraw();
private:
	virtual void WorkOutDefaultScale();
	virtual void WorkOutDefaultOffset();

	sf::RenderWindow *m_pSfmlWindow = nullptr;
};

#endif // !SFML_SPINE_PLAYER_H_
