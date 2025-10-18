#ifndef SFML_SPINE_PLAYER_H_
#define SFML_SPINE_PLAYER_H_

#include"spine_player.h"

class CSfmlSpinePlayer : public CSpinePlayer
{
public:
	CSfmlSpinePlayer();
	virtual ~CSfmlSpinePlayer();

	void Draw(sf::RenderTarget* renderTarget);
private:
	virtual void WorkOutDefaultScale();
	virtual void WorkOutDefaultOffset();
};

#endif // !SFML_SPINE_PLAYER_H_
