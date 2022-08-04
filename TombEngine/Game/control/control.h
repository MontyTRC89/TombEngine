#pragma once
#include "Game/animation.h"
#include "Game/control/trigger.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Specific/phd_global.h"

struct BOUNDING_BOX;
struct ItemInfo;
struct CollisionInfo;
class FloorInfo;
struct ANIM_STRUCT;
struct MESH_INFO;
struct ROOM_INFO;

enum class GameStatus
{
	None,
	NewGame,
	LoadGame,
	SaveGame,
	ExitToTitle,
	ExitGame,
	LaraDead,
	LevelComplete
};

enum HEADINGS
{
	NORTH,
	EAST,
	SOUTH,
	WEST
};

enum FADE_STATUS
{
	FADE_STATUS_NONE,
	FADE_STATUS_FADEIN,
	FADE_STATUS_FADEOUT
};

constexpr int MAX_ROOMS = 1024;

constexpr int WIBBLE_SPEED = 4;
constexpr int WIBBLE_MAX = UCHAR_MAX - WIBBLE_SPEED + 1;

extern int GameTimer;
extern int RumbleTimer;
extern int GlobalCounter;
extern int Wibble;

extern bool InitialiseGame;
extern bool DoTheGame;
extern bool JustLoaded;
extern bool ThreadEnded;

extern int RequiredStartPos;
extern int CurrentLevel;
extern int LevelComplete;
extern int SystemNameHash;

extern bool  InItemControlLoop;
extern short ItemNewRoomNo;
extern short ItemNewRooms[MAX_ROOMS];
extern short NextItemActive;
extern short NextItemFree;
extern short NextFxActive;
extern short NextFxFree;

extern std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];

int DrawPhase();

GameStatus DoTitle(int index, std::string const & ambient);
GameStatus DoLevel(int index, std::string const & ambient, bool loadFromSavegame);
GameStatus ControlPhase(int numFrames, int demoMode);

int GetRandomControl();
int GetRandomDraw();

void KillMoveItems();
void KillMoveEffects();
void UpdateShatters();

void CleanUp();

unsigned CALLBACK GameMain(void*);
