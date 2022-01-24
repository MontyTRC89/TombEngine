#pragma once
#include "Game/animation.h"
#include "Game/control/trigger.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Specific/phd_global.h"

struct BOUNDING_BOX;
struct ITEM_INFO;
struct COLL_INFO;
class FLOOR_INFO;
struct ANIM_STRUCT;
struct MESH_INFO;
struct ROOM_INFO;

enum class GAME_STATUS
{
	GAME_STATUS_NONE,
	GAME_STATUS_NEW_GAME,
	GAME_STATUS_LOAD_GAME,
	GAME_STATUS_SAVE_GAME,
	GAME_STATUS_EXIT_TO_TITLE,
	GAME_STATUS_EXIT_GAME,
	GAME_STATUS_LARA_DEAD,
	GAME_STATUS_LEVEL_COMPLETED
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

extern bool  InItemControlLoop;
extern short ItemNewRoomNo;
extern short ItemNewRooms[MAX_ROOMS];
extern short NextItemActive;
extern short NextItemFree;
extern short NextFxActive;
extern short NextFxFree;

extern int WeaponDelay;
extern int WeaponEnemyTimer;

extern std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];

int DrawPhase();

GAME_STATUS DoTitle(int index);
GAME_STATUS DoLevel(int index, std::string ambient, bool loadFromSavegame);
GAME_STATUS ControlPhase(int numFrames, int demoMode);

int GetRandomControl();
int GetRandomDraw();

void KillMoveItems();
void KillMoveEffects();
void UpdateShatters();
int ExplodeItemNode(ITEM_INFO* item, int Node, int NoXZVel, int bits);

void CleanUp();

void AlterFloorHeight(ITEM_INFO* item, int height);
int GetFloorHeight(FLOOR_INFO* floor, int x, int y, int z);
FLOOR_INFO* GetFloor(int x, int y, int z, short* roomNumber);
int GetCeiling(FLOOR_INFO* floor, int x, int y, int z);	
int GetWaterSurface(int x, int y, int z, short roomNumber);
int GetWaterDepth(int x, int y, int z, short roomNumber);
int GetWaterHeight(int x, int y, int z, short roomNumber);
int GetDistanceToFloor(int itemNumber, bool precise = true);

unsigned CALLBACK GameMain(void*);