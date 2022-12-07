#pragma once
#include "Game/animation.h"
#include "Game/control/trigger.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Math/Math.h"

class FloorInfo;
class GameBoundingBox;
struct AnimData;
struct CollisionInfo;
struct ItemInfo;
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

int DrawPhase(bool title);

GameStatus ControlPhase(int numFrames);
GameStatus DoLevel(int levelIndex, bool loadGame = false);
GameStatus DoGameLoop(int levelIndex);
void EndGameLoop(int levelIndex);

GameStatus HandleMenuCalls(bool title);
GameStatus HandleGlobalInputEvents(bool title);
void HandleControls(bool title);

int GetRandomControl();
int GetRandomDraw();

void KillMoveItems();
void KillMoveEffects();
void UpdateShatters();

void CleanUp();

void InitialiseOrLoadGame(bool loadGame);
void InitialiseScripting(int levelIndex, bool loadGame);
void DeInitialiseScripting(int levelIndex);

unsigned CALLBACK GameMain(void*);
