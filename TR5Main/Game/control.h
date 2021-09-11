#pragma once

#include "Specific\phd_global.h"
#include "items.h"
#include "room.h"
#include "control\trigger.h"

struct BOUNDING_BOX;
struct ITEM_INFO;
struct COLL_INFO;
class FLOOR_INFO;
struct ANIM_STRUCT;
struct MESH_INFO;
struct ROOM_INFO;
enum GAME_STATUS
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

enum COMMAND_TYPES
{
	COMMAND_NULL = 0,
	COMMAND_MOVE_ORIGIN,
	COMMAND_JUMP_VELOCITY,
	COMMAND_ATTACK_READY,
	COMMAND_DEACTIVATE,
	COMMAND_SOUND_FX,
	COMMAND_EFFECT
};

#define TRIG_BITS(T) ((T & 0x3FFF) >> 10)

#define OUTSIDE_Z 64
#define OUTSIDE_SIZE 108

extern int KeyTriggerActive;
extern byte FlipStatus;

constexpr auto MAX_FLIPMAP = 255;
extern int FlipStats[MAX_FLIPMAP];
extern int FlipMap[MAX_FLIPMAP];

extern bool InItemControlLoop;
extern short ItemNewRoomNo;
extern short ItemNewRooms[512];
extern short NextFxActive;
extern short NextFxFree;
extern short NextItemActive;
extern short NextItemFree;
extern int DisableLaraControl;
extern int WeatherType;
extern int LaraDrawType;
extern int NumAnimatedTextures;
extern short* AnimTextureRanges;
extern int nAnimUVRanges;
extern int Wibble;
extern int SetDebounce;
extern std::string CurrentAtmosphere;
extern short CurrentRoom;
extern int GameTimer;
extern short GlobalCounter;
extern byte LevelComplete;
#ifndef NEW_INV
extern int LastInventoryItem;
#endif
extern int TrackCameraInit;
extern short TorchRoom;
extern int InitialiseGame;
extern int RequiredStartPos;
extern int WeaponDelay;
extern int WeaponEnemyTimer;
extern short SkyPos1;
extern short SkyPos2;
extern CVECTOR SkyColor1;
extern CVECTOR SkyColor2;
extern int CutSeqNum;
extern int CurrentLevel;
extern bool DoTheGame;
extern bool ThreadEnded;
extern int OnFloor;
extern int SmokeWindX;
extern int SmokeWindZ;
extern int FlipTimer;
extern int FlipEffect;
extern int TriggerTimer;
extern int JustLoaded;
extern int OldLaraBusy;
extern int Infrared;
extern short FlashFadeR;
extern short FlashFadeG;
extern short FlashFadeB;
extern short FlashFader;
extern std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];
extern short IsRoomOutsideNo;

GAME_STATUS DoTitle(int index);
GAME_STATUS DoLevel(int index, std::string ambient, bool loadFromSavegame);
GAME_STATUS ControlPhase(int numFrames, int demoMode);
void UpdateSky();
void AnimateWaterfalls();
void TranslateItem(ITEM_INFO* item, int x, int y, int z);
int GetWaterSurface(int x, int y, int z, short roomNumber);
void KillMoveItems();
void KillMoveEffects();
int GetChange(ITEM_INFO* item, ANIM_STRUCT* anim);
void AlterFloorHeight(ITEM_INFO* item, int height);
int CheckNoColCeilingTriangle(FLOOR_INFO* floor, int x, int z);
int CheckNoColFloorTriangle(FLOOR_INFO* floor, int x, int z);
int GetFloorHeight(FLOOR_INFO* floor, int x, int y, int z);
FLOOR_INFO* GetFloor(int x, int y, int z, short* roomNumber);
//void UpdateDebris();
int LOS(GAME_VECTOR* start, GAME_VECTOR* end);
int xLOS(GAME_VECTOR* start, GAME_VECTOR* end);
int zLOS(GAME_VECTOR* start, GAME_VECTOR* end);
int ClipTarget(GAME_VECTOR* start, GAME_VECTOR* target);
int GetTargetOnLOS(GAME_VECTOR* src, GAME_VECTOR* dest, int DrawTarget, int firing);
int ObjectOnLOS2(GAME_VECTOR* start, GAME_VECTOR* end, PHD_VECTOR* vec, MESH_INFO** mesh);
int GetRandomControl();
int GetRandomDraw();
int GetCeiling(FLOOR_INFO* floor, int x, int y, int z);
int DoRayBox(GAME_VECTOR* start, GAME_VECTOR* end, BOUNDING_BOX* box, PHD_3DPOS* itemOrStaticPos, PHD_VECTOR* hitPos, short closesItemNumber);
void AnimateItem(ITEM_INFO* item);
void DoFlipMap(short group);
void AddRoomFlipItems(ROOM_INFO* r);
void RemoveRoomFlipItems(ROOM_INFO* r);
void PlaySoundTrack(short track, short flags);
void RumbleScreen();
void RefreshCamera(short type, short* data);
int ExplodeItemNode(ITEM_INFO* item, int Node, int NoXZVel, int bits);
int GetWaterHeight(int x, int y, int z, short roomNumber);
int is_object_in_room(short roomNumber, short objectNumber);
void InterpolateAngle(short angle, short* rotation, short* outAngle, int shift);
int IsRoomOutside(int x, int y, int z);
void ResetGlobals();

unsigned CALLBACK GameMain(void*);
