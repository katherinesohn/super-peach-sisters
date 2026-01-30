#include "Actor.h"
#include "StudentWorld.h"
#include <iostream>
using namespace std;

//switch the direction of an actor
void Actor::switchDirection()
{
	if (getDirection() == 0)
		setDirection(180);
	else if (getDirection() == 180)
		setDirection(0);
}

//execute a tick for Peach
void Peach::doSomething()
{
	//1. check if peach is currently alive, if not return 
	if (!isAlive())
		return;

	//check star power invincibility, decrement, check temp invincibility likewise
	if (m_starPower > 0)
		m_starPower--;

	if (m_tempInvincibility > 0)
		m_tempInvincibility--;

	//check recharge time 
	if (m_rechargeTime > 0)
		m_rechargeTime--;

	//actively jumping upwards
	if (m_remainingJumpDistance > 0)
	{ 
		//if blocking object, bonk and terminate jump
		if (getWorld()->blockingActorAt(getX(), getY() + 4) != nullptr)
		{
			getWorld()->blockingActorAt(getX(), getY() + 4)->bonk();
			m_remainingJumpDistance = 0;
		}
		else
		{
			//no blocking object
			moveTo(getX(), getY() + 4);
			m_remainingJumpDistance--;
		}
	}
	//checks if she is falling
	else
	{
		//if no blocking object, fall 
		if (getWorld()->blockingActorAt(getX(), getY() - 4) == nullptr)
			moveTo(getX(), getY() - 4);
	}

	//check if player has pressed a key  
	int keyPressed;
	if (getWorld()->getKey(keyPressed))
	{
		if (keyPressed == KEY_PRESS_LEFT)
		{
			setDirection(180);

			//check 4 pixels to the left of left edge of sprite
			if (getWorld()->blockingActorAt(getX() - 4, getY()) == nullptr)
			{
				moveTo(getX() - 4, getY());
			}
				
		}
		else if (keyPressed == KEY_PRESS_RIGHT)
		{
			setDirection(0);

			//check 4 pixels to the right of right edge of sprite
			if (getWorld()->blockingActorAt(getX() + 4, getY()) == nullptr)
				moveTo(getX() + 4, getY());

			//cerr << "peach left: " << getX() << endl;
			//cerr << "peach right: " << getY() << endl;

		}
		else if (keyPressed == KEY_PRESS_UP)
		{
			//check for jump power 
			if (getWorld()->blockingActorAt(getX(), getY() - 1) != nullptr)
			{
				if (!m_hasJumpPower)
					m_remainingJumpDistance = 8;
				else
					m_remainingJumpDistance = 12;

				getWorld()->playSound(SOUND_PLAYER_JUMP);
			}
		}
		else if (keyPressed == KEY_PRESS_SPACE)
		{
			//if no shoot power or time to recharge is greater than zero, do nothing
			if (!m_hasShootPower || m_rechargeTime > 0)
				return;
			
			//otherwise play sound, set recharge time, make a fireball 
			getWorld()->playSound(SOUND_PLAYER_FIRE);

			int shift = 4;
			if (getDirection() == 180)
				shift *= -1;

			getWorld()->addActor(new PeachFireball(getX() + shift, getY(), getWorld(), getDirection()));

			m_rechargeTime = 8; 
		}
	}
}

//bonk peach 
void Peach::bonk()
{
	//do nothing if she is invincible 
	if (m_starPower > 0 || m_tempInvincibility > 0)
		return; 

	m_hp--;
	m_tempInvincibility = 10; 

	if (m_hasShootPower)
		m_hasShootPower = false;
	if (m_hasJumpPower)
		m_hasJumpPower = false;

	if (m_hp > 0)
		getWorld()->playSound(SOUND_PLAYER_HURT);
	else
		setDead();
}

/////////////////////////////////
///////		STRUCTURES
/////////////////////////////////

//bonk a block 
void Block::bonk()
{
	//doesn't hold a goodie or has alr released goodie
	if (m_goodieType == 0 || bonkedThisLevel)
	{
		getWorld()->playSound(SOUND_PLAYER_BONK);
		return;
	}

	//does hold a goodie and hasn't been bonked yet
	//set goodie type to 0 to indicate already released
	getWorld()->playSound(SOUND_POWERUP_APPEARS);
		
	switch (m_goodieType)
	{
	case 1:
	{
		getWorld()->addActor(new Star(getX(), getY() + 8, getWorld()));
		break;
	}
	case 2:
	{
		getWorld()->addActor(new Flower(getX(), getY() + 8, getWorld()));
		m_goodieType = 0;
		break;
	}
	case 3:
	{
		getWorld()->addActor(new Mushroom(getX(), getY() + 8, getWorld()));
		m_goodieType = 0;
		break;
	}
	}

		
	bonkedThisLevel = true;
}

/////////////////////////////////
///////		LEVELENDR
/////////////////////////////////

//execute a tick for levelEndr
void LevelEndr::doSomething()
{
	if (!isAlive())
		return;
	
	//if overlapping peach, tell StudentWorld that level/game is over
	if (getWorld()->overlapsPeach(this))
	{
		getWorld()->increaseScore(1000);
		setDead();
		endFunction(); //different for mario (end game instead of level)
	}
}


/////////////////////////////////
///////		GOODIES
/////////////////////////////////

//excecute a tick for a goodie
void Goodie::doSomething()
{
	//if overlapping peach, give points and health if it does, kill itself and play sound
	if (getWorld()->overlapsPeach(this))
	{
		getWorld()->increaseScore(m_pointVal); 

		givePower(); 

		if (m_givesHealth)
			getWorld()->getPeach()->setHitPoints(2);

		setDead();

		getWorld()->playSound(SOUND_PLAYER_POWERUP);
		
		return;
	}
	
	if (getWorld()->blockingActorAt(getX(), getY() - 2) == nullptr)
	{
		moveTo(getX(), getY() - 2);
	}

	int move = 2;
	if (getDirection() == 180)
		move = -2;

	//if there's a blocking object, reverse direction and return
	if (getWorld()->blockingActorAt(getX() + move, getY()) != nullptr)
	{
		switchDirection();
		return;
	}
	else
		moveTo(getX() + move, getY());

}

/////////////////////////////////
///////		PROJECTILES
/////////////////////////////////

//excecute a tick for a projectile
void Projectile::doSomething()
{
	//if it overlaps, damages and kills itself
	if (overlaps())
		return;
	
	//else move based on direction 
	if (getWorld()->blockingActorAt(getX(), getY() - 2) == nullptr)
		moveTo(getX(), getY() - 2);
	
	int shift = 2;
	if (getDirection() == 180)
		shift *= -1;

	//dies when it hits a blocking actor
	if (getWorld()->blockingActorAt(getX() + shift, getY()) != nullptr)
	{
		setDead();
		return;
	}
	else
		moveTo(getX() + shift, getY());
}

//checks for something to damage, default checks non-peaches (shells and peachFireballs)
bool Projectile::overlaps()
{
	Actor* a = getWorld()->damageableOverlap(this);
	if (a != nullptr && a->isAlive())
	{
		//a is damageable by this point
		a->bonk();
		setDead();
		return true;
	}
	return false;
}

//checks overlap with peach instead of non-peaches 
bool PiranhaFireball::overlaps()
{
	if (getWorld()->overlapsPeach(this))
	{
		getWorld()->getPeach()->bonk();
		setDead();
		return true;
	}
	return false;
}

/////////////////////////////////
///////		ENEMIES
/////////////////////////////////

//written because piranha shares this with enemies but not the rest of doSomething()
void Enemy::checkPeachOverlap()
{
	//if it overlaps with peach, bonk peach and return
	if (getWorld()->overlapsPeach(this))
	{
		if (getWorld()->getPeach()->hasStarPower())
		{
			getWorld()->playSound(SOUND_PLAYER_KICK);
			bonk();
		}

		getWorld()->getPeach()->bonk();
		return;
	}
}

//executes a tick for an enemy
void Enemy::doSomething()
{
	//if enemy is not alive, return immediately
	if (!isAlive())
		return;

	checkPeachOverlap();

	//if facing right, switch dir if blocking object or edge 
	if (getDirection() == 0)
	{
		if (getWorld()->blockingActorAt(getX() + 1, getY()) != nullptr ||
			getWorld()->blockingActorAt(getX() + SPRITE_WIDTH - 1 + 1, getY() - 1) == nullptr)
			switchDirection();
	}
	else //direction == 180 left
	{
		if (getWorld()->blockingActorAt(getX() - 1, getY()) != nullptr || 
			getWorld()->blockingActorAt(getX() - SPRITE_WIDTH + 1 - 1, getY() - 1) == nullptr)
			switchDirection();
	}

	int move = 1;
	if (getDirection() == 180)
		move = -1;

	//new direction, move 
	if (getWorld()->blockingActorAt(getX() + move, getY()) != nullptr)
		return;
	else
		moveTo(getX() + move, getY());
}

//when damaged, give score and die (default)
void Enemy::bonk()
{
	getWorld()->increaseScore(100);
	setDead();
}

//when damaged, same thing but also make a shell
void Koopa::bonk()
{
	//don't keep checking for overlap etc
	if (!isAlive())
		return;

	getWorld()->increaseScore(100);
	setDead();
	getWorld()->addActor(new Shell(getX(), getY(), getWorld(), getDirection()));
}

//execute a tick for a piranha -- doesn't move 
void Piranha::doSomething()
{
	if (!isAlive())
		return;

	increaseAnimationNumber();

	checkPeachOverlap();

	int pY = getWorld()->getPeach()->getY();
	int pX = getWorld()->getPeach()->getX();

	//piranha only does anything else if peach is on the same level
	if (getY() - 1.5 * SPRITE_HEIGHT < pY && pY < getY() + 1.5 * SPRITE_HEIGHT)
	{

		if (pX < getX()) //peach is to its left
			setDirection(180);
		else
			setDirection(0);

		if (m_firingDelay > 0)
		{
			m_firingDelay--;
			return;
		}
		//if no firing delay, can choose to fire if peach is in range
		if (getX() - 8 * SPRITE_WIDTH < pX && pX < getX() + 8 * SPRITE_WIDTH)
		{
			getWorld()->addActor(new PiranhaFireball(getX(), getY(), getWorld(), getDirection()));
			getWorld()->playSound(SOUND_PIRANHA_FIRE);
			m_firingDelay = 40;
		}
	}
}
