#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
    return new StudentWorld(assetPath);
}

//constructor 
StudentWorld::StudentWorld(string assetPath)
    : GameWorld(assetPath), peach(nullptr), m_levelOver(false), m_gameOver(false)
{
}

//intialize everything, read level file 
int StudentWorld::init()
{
    m_levelOver = false;
    m_gameOver = false;

    /////////////////////////////////
    //LEVEL FILE READ 
    /////////////////////////////////

    Level lev(assetPath());
    ostringstream level_file;
    level_file << "level" << setw(2) << setfill('0') << getLevel() << ".txt";
    cerr << level_file.str() << endl;

    Level::LoadResult result = lev.loadLevel(level_file.str());

    //check bad file reads
    if (result == Level::load_fail_file_not_found)
        return GWSTATUS_LEVEL_ERROR;
    else if (result == Level::load_fail_bad_format)
        return GWSTATUS_LEVEL_ERROR;
    
    //read in level txt file 
    else if (result == Level::load_success)
    {
        cerr << "Successfully loaded level" << endl;

        for (int lx = 0; lx < GRID_WIDTH; lx++)
        {
            for (int ly = 0; ly < GRID_HEIGHT; ly++)
            {
                Level::GridEntry ge;
                ge = lev.getContentsOf(lx, ly);

                switch (ge)
                {
                case Level::empty:
                    //cout << "Location " << lx << ", " << ly << " is empty" << endl;
                    break;
                case Level::peach:
                    //cout << "Location " << lx << ", " << ly << " is where Peach starts" << endl;
                    peach = new Peach(lx * SPRITE_WIDTH, ly * SPRITE_HEIGHT, this);
                    break;
                case Level::koopa:
                    //cout << "Location " << lx << ", " << ly << " starts with a koopa" << endl;
                    a.push_back(new Koopa(lx * SPRITE_WIDTH, ly * SPRITE_HEIGHT, this));
                    break;
                case Level::goomba:
                    //cout << "Location " << lx << ", " << ly << " starts with a goomba" << endl;
                    a.push_back(new Goomba(lx * SPRITE_WIDTH, ly * SPRITE_HEIGHT, this));
                    break;
                case Level::piranha:
                    //cout << "Location " << lx << ", " << ly << " starts with a piranha" << endl;
                    a.push_back(new Piranha(lx * SPRITE_WIDTH, ly * SPRITE_HEIGHT, this));
                    break;
                case Level::flag:
                    //cout << "Location " << lx << ", " << ly << " is where a flag is" << endl;
                    a.push_back(new Flag(lx * SPRITE_WIDTH, ly * SPRITE_HEIGHT, this));
                    break;
                case Level::block:
                    //cout << "Location " << lx << ", " << ly << " holds a regular block" << endl;
                    a.push_back(new Block(lx * SPRITE_WIDTH, ly * SPRITE_HEIGHT, this, 0));
                    break;
                case Level::star_goodie_block:
                    //cout << "Location " << lx << ", " << ly << " has a star goodie block " << endl;
                    a.push_back(new Block(lx * SPRITE_WIDTH, ly * SPRITE_HEIGHT, this, 1));
                    break;
                case Level::flower_goodie_block:
                    //cout << "Location " << lx << ", " << ly << " has a flower goodie block " << endl;
                    a.push_back(new Block(lx * SPRITE_WIDTH, ly * SPRITE_HEIGHT, this, 2));
                    break;
                case Level::mushroom_goodie_block:
                    //cout << "Location " << lx << ", " << ly << " has a mushroom goodie block " << endl;
                    a.push_back(new Block(lx * SPRITE_WIDTH, ly * SPRITE_HEIGHT, this, 3));
                    break;
                case Level::pipe:
                    //cout << "Location " << lx << ", " << ly << " holds a pipe block" << endl;
                    a.push_back(new Pipe(lx * SPRITE_WIDTH, ly * SPRITE_HEIGHT, this));
                    break;
                case Level::mario:
                    //cout << "Location " << lx << ", " << ly << " holds ma rio" << endl;
                    a.push_back(new Mario(lx * SPRITE_WIDTH, ly * SPRITE_HEIGHT, this));
                    break;
                }
            }
        }
    }


    //INITIALIZE DISPLAY TEXT
    updateDisplay();


    return GWSTATUS_CONTINUE_GAME;
}

//executes a tick
int StudentWorld::move()
{
    if(peach != nullptr)
        peach->doSomething();

    //execute each actor other than peach
    for (int i = 0; i < a.size(); i++)
    {
        if (a[i] != nullptr)
        {
            a[i]->doSomething();

            if (!peach->isAlive())
            {
                decLives();
                playSound(SOUND_PLAYER_DIE);
                return GWSTATUS_PLAYER_DIED;
            }

            if (m_gameOver)
            {
                playSound(SOUND_GAME_OVER);
                return GWSTATUS_PLAYER_WON;
            }

            if (m_levelOver)
            {
                playSound(SOUND_FINISHED_LEVEL);
                return GWSTATUS_FINISHED_LEVEL;
            }
        }

    }

    //remove newly-dead actors 
    for (vector<Actor*>::iterator i = a.begin(); i < a.end();)
        if (!(*i)->isAlive())
        {
            Actor* temp = *i;
            delete temp;
            i = a.erase(i);
        }
        else
            i++;

    updateDisplay();

    return GWSTATUS_CONTINUE_GAME;
}

//delete all actors and peach
void StudentWorld::cleanUp()
{
    for (int i = 0; i < a.size(); i++)
    {
        delete a[i];
    }

    delete peach;

    a.clear();
}


//returns pointer to a2 if the two overlap, otherwise nullptr
Actor* StudentWorld::overlap(Actor* a1, Actor* a2) const
{
    int a1Left = a1->getX();
    int a1Right = a1Left + SPRITE_WIDTH - 1;
    int a1Bot = a1->getY();
    int a1Top = a1Bot + SPRITE_HEIGHT - 1;

    int a2Left = a2->getX();
    int a2Right = a2Left + SPRITE_WIDTH - 1;
    int a2Bot = a2->getY();
    int a2Top = a2Bot + SPRITE_HEIGHT - 1;

    if ((a2Left >= a1Left && a2Left <= a1Right && a2Top >= a1Bot && a2Top <= a1Top) ||
        (a1Left >= a2Left && a1Left <= a2Right && a2Top >= a1Bot && a2Top <= a1Top) ||
        (a1Right >= a2Left && a1Right <= a2Right && a1Top >= a2Bot && a1Top <= a2Top) ||
        (a2Right >= a1Left && a2Right <= a1Right && a1Top >= a2Bot && a1Top <= a2Top))
        return a2;


    return nullptr;
}

//returns true if parameter actor overlaps with peach
bool StudentWorld::overlapsPeach(Actor* a) const
{
    if (overlap(a, peach) == peach)
        return true;

    return false;
}

//ignores peach, returns actor if it overlaps with parameter, else nullptr
Actor* StudentWorld::damageableOverlap(Actor* a1) const
{
    for (int i = 0; i < a.size(); i++)
    {
        //don't check if it overlaps with itself
        if (a[i] != a1)
            if (overlap(a1, a[i]) != nullptr && a[i]->isDamageable())
                return a[i];
    }

    return nullptr;
}

//ignores peach, returns a[i] that collides with box from x, y
Actor* StudentWorld::blockingActorAt(int x, int y) const
{
    //for each actor, see if it overlaps 
    for (int i = 0; i < a.size(); i++)
    {
        //only check potentially blocking actors
        if (a[i]->isPassable() == false)
        {
            int a1Left = x;
            int a1Right = a1Left + SPRITE_WIDTH - 1;
            int a1Bot = y;
            int a1Top = a1Bot + SPRITE_HEIGHT - 1;


            int a2Left = a[i]->getX();
            int a2Right = a2Left + SPRITE_WIDTH - 1;
            int a2Bot = a[i]->getY();
            int a2Top = a2Bot + SPRITE_HEIGHT - 1;


            if ((a2Left >= a1Left && a2Left <= a1Right && a2Top >= a1Bot && a2Top <= a1Top) ||
                (a1Left >= a2Left && a1Left <= a2Right && a2Top >= a1Bot && a2Top <= a1Top) ||
                (a1Right >= a2Left && a1Right <= a2Right && a1Top >= a2Bot && a1Top <= a2Top) ||
                (a2Right >= a1Left && a2Right <= a1Right && a1Top >= a2Bot && a1Top <= a2Top))
                return a[i];
           
        }
    }

    return nullptr;
}

//update display text 
void StudentWorld::updateDisplay()
{
    ostringstream displayText;
    displayText.fill('0');
    displayText << "Lives: " << getLives() << "  Level: " << getLevel() << "  Points: " << setw(6) << getScore();
    
    if (getPeach()->hasStarPower())
        displayText << " StarPower!";
    if (getPeach()->hasShootPower())
        displayText << " ShootPower!";
    if (getPeach()->hasJumpPower())
        displayText << " JumpPower!";

    setGameStatText(displayText.str());
}

