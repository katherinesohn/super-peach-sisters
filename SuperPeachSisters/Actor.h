#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include <string> 
#include <stdlib.h>
#include <ctime>
#include <stdlib.h>
#include "StudentWorld.h"
using namespace std;


////////////////////////////////////////
///			ACTOR
////////////////////////////////////////

class Actor : public GraphObject
{
public:
	Actor(int imageID, int startX, int startY, int dir, int depth, double size, bool alive,
		StudentWorld* world, bool p, bool d)
		: GraphObject(imageID, startX, startY, dir, depth, size), m_alive(alive), m_world(world),
		m_passable(p), m_damageable(d)
	{
	}

	virtual void doSomething() = 0;
	virtual void bonk() = 0; 
	StudentWorld* getWorld() const { return m_world; }
	bool isPassable() const { return m_passable; }
	bool isDamageable() const { return m_damageable; }
	bool isAlive() const { return m_alive; }
	void setDead() { m_alive = false; }
	void switchDirection();


private:
	bool m_alive;
	StudentWorld* m_world;
	bool m_passable;
	bool m_damageable;
};

////////////////////////////////////////
///			PEACH
////////////////////////////////////////

class Peach : public Actor
{
public:
	Peach(int x, int y, StudentWorld* world)
		: Actor(IID_PEACH, x, y, 0, 0, 1.0, true, world, true, true), m_hp(1),
		m_starPower(0), m_hasShootPower(false), m_hasJumpPower(false), m_remainingJumpDistance(0), m_rechargeTime(0),
		m_tempInvincibility(0)
	{}

	virtual void doSomething();
	virtual void bonk();
	void getStarPower(int ticks) { m_starPower = ticks; }
	void getJumpPower() { m_hasJumpPower = true; }
	void getShootPower() { m_hasShootPower = true; }
	void setHitPoints(int hp){ m_hp = hp; }
	bool hasStarPower() const { return m_starPower > 0; } //only used for display 
	bool hasJumpPower() const { return m_hasJumpPower; }
	bool hasShootPower() const { return m_hasShootPower; }

private:
	int m_hp;
	int m_tempInvincibility;
	int m_starPower;
	bool m_hasShootPower;
	bool m_hasJumpPower;
	int m_rechargeTime;
	int m_remainingJumpDistance;
};

////////////////////////////////////////////
///////			GOODIES 
////////////////////////////////////////////
class Goodie : public Actor
{
public: 
	Goodie(int x, int y, int iid, StudentWorld* world, int pT, int pV, bool gH)
		: Actor(iid, x, y, 0, 1, 1.0, true, world, true, false),
		m_powerType(pT), m_pointVal(pV), m_givesHealth(gH)
	{}
	virtual void doSomething();
	virtual void bonk() {};
private:
	int m_powerType;
	int m_pointVal; 
	bool m_givesHealth;
	virtual void givePower() = 0;
};

class Star : public Goodie
{
public:
	Star(int x, int y, StudentWorld* world)
		: Goodie(x, y, IID_STAR, world, 1, 100, false)
	{}
private:
	virtual void givePower() { getWorld()->getPeach()->getStarPower(150); }
};


class Flower : public Goodie
{
public:
	Flower(int x, int y, StudentWorld* world)
		: Goodie(x, y, IID_FLOWER, world, 2, 50, true)
	{}
private:
	virtual void givePower() { getWorld()->getPeach()->getShootPower(); }
};

class Mushroom : public Goodie
{
public:
	Mushroom(int x, int y, StudentWorld* world)
		: Goodie(x, y, IID_MUSHROOM, world, 3, 75, true)
	{}
private:
	virtual void givePower() { getWorld()->getPeach()->getJumpPower(); }
};


////////////////////////////////////////
///			STRUCTURES
////////////////////////////////////////


class Structure : public Actor
{
public:
	Structure(int x, int y, int iid, StudentWorld* world)
		: Actor(iid, x, y, 0, 2, 1.0, true, world, false, false)
	{}
	virtual void doSomething() {}; //structures do nothing during a tick
	virtual void bonk() = 0;
private:

};

class Block : public Structure
{
public:
	Block(int x, int y, StudentWorld* world, int goodie)
		: Structure(x, y, IID_BLOCK, world), m_goodieType(goodie), m_goodie(nullptr), bonkedThisLevel(false)
	{}
	virtual void bonk();
private:
	int m_goodieType; // 0 - none, 1 - star, 2 - flower, 3 - mushroom
	Goodie* m_goodie;
	bool bonkedThisLevel;
};

class Pipe : public Structure
{
public:
	Pipe(int x, int y, StudentWorld* world)
		: Structure(x, y, IID_PIPE, world)
	{}
	virtual void bonk() {};
};

/////////////////////////////////
///////		LEVELENDR
/////////////////////////////////

class LevelEndr : public Actor
{
public:
	LevelEndr(int x, int y, int iid, StudentWorld* world)
		: Actor(iid, x, y, 0, 1, 1, true, world, true, false)
	{}
	virtual void doSomething();
	virtual void endFunction() = 0;
	virtual void bonk() {};
};

class Flag : public LevelEndr
{
public:
	Flag(int x, int y, StudentWorld* world)
		: LevelEndr(x, y, IID_FLAG, world)
	{}
	virtual void endFunction() { getWorld()->endLevel(); }
};

class Mario : public LevelEndr
{
public:
	Mario(int x, int y, StudentWorld* world)
		: LevelEndr(x, y, IID_MARIO, world)
	{}
	virtual void endFunction() { getWorld()->endGame(); }
};


////////////////////////////////////////
///			PROJECTILES
////////////////////////////////////////

class Projectile : public Actor
{
public:
	Projectile(int x, int y, int iid, StudentWorld* world, int dir)
		: Actor(iid, x, y, dir, 1, 1, true, world, true, false)
	{}
	virtual void doSomething();
	virtual void bonk() {};
private:
	virtual bool overlaps();
};

class PiranhaFireball : public Projectile
{
public: 
	PiranhaFireball(int x, int y, StudentWorld* world, int dir)
		: Projectile(x, y, IID_PIRANHA_FIRE, world, dir)
	{}
private:
	virtual bool overlaps(); //checks with peach instead of non-peach objects
};

class PeachFireball : public Projectile
{
public:
	PeachFireball(int x, int y, StudentWorld* world, int dir)
		: Projectile(x, y, IID_PEACH_FIRE, world, dir)
	{}
};

class Shell : public Projectile
{
public:
	Shell(int x, int y, StudentWorld* world, int dir)
		: Projectile(x, y, IID_SHELL, world, dir)
	{}
};


////////////////////////////////////////
///			ENEMIES
////////////////////////////////////////

class Enemy : public Actor
{
public:
	Enemy(int x, int y, int iid, StudentWorld* world)
		: Actor(iid, x, y, 0, 0, 1, true, world, true, true)
	{
		srand(time(NULL));
		if (rand() % 2 == 1)
			setDirection(180);
	}
	virtual void doSomething();
	virtual void checkPeachOverlap();
	virtual void bonk();
};

class Goomba : public Enemy
{
public:
	Goomba(int x, int y, StudentWorld* world)
		: Enemy(x, y, IID_GOOMBA, world)
	{}
};

class Koopa : public Enemy
{
public:
	Koopa(int x, int y, StudentWorld* world)
		: Enemy(x, y, IID_KOOPA, world)
	{}
	virtual void bonk(); 
};

class Piranha : public Enemy
{
public:
	Piranha(int x, int y, StudentWorld* world)
		: Enemy(x, y, IID_PIRANHA, world), m_firingDelay(0)
	{}
	virtual void doSomething();

private: 
	int m_firingDelay; 
};
#endif // ACTOR_H_


