#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include <vector>
#include <string>
#include "GameWorld.h"
#include "Level.h"
class Actor;
class Peach;

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetPath);
	~StudentWorld() { cleanUp(); };
	virtual int init();
	virtual int move();
	virtual void cleanUp();
	Actor* overlap(Actor* a1, Actor* a2) const;
	bool overlapsPeach(Actor* a) const;
	Actor* damageableOverlap(Actor* a1) const;
	Actor* blockingActorAt(int x, int y) const;
	Peach* getPeach() const { return peach; }
	void addActor(Actor* a1) { if(a1 != nullptr) a.push_back(a1); }
	void endLevel() { m_levelOver = true; }
	void endGame() { m_gameOver = true; }

private:
	std::vector<Actor*> a;
	Peach* peach;
	bool m_levelOver;
	bool m_gameOver;
	void updateDisplay();
};

#endif // STUDENTWORLD_H_
