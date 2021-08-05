#pragma once
#include "phd_global.h"
#include "items.h"
#include "room.h"

struct BOUNDING_BOX;

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

enum COLL_TYPE
{
	CT_NONE = 0,				// 0x00
	CT_FRONT = (1 << 0),		// 0x01
	CT_LEFT = (1 << 1),			// 0x02
	CT_RIGHT = (1 << 2),		// 0x04
	CT_TOP = (1 << 3),			// 0x08
	CT_TOP_FRONT = (1 << 4),	// 0x10
	CT_CLAMP = (1 << 5)			// 0x20
};

enum HEIGHT_TYPES
{
	WALL,
	SMALL_SLOPE,
	BIG_SLOPE,
	DIAGONAL,
	SPLIT_TRI
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
extern byte IsAtmospherePlaying;
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
extern short* TriggerIndex;
extern int DisableLaraControl;
extern int WeatherType;
extern int LaraDrawType;
extern int NumAnimatedTextures;
extern short* AnimTextureRanges;
extern int nAnimUVRanges;
extern int Wibble;
extern int SetDebounce;
extern std::string CurrentAtmosphere;
extern short ShatterSounds[18][10];
extern short CurrentRoom;
extern int GameTimer;
extern short GlobalCounter;
extern byte LevelComplete;
extern short DelCutSeqPlayer;
#ifndef NEW_INV
extern int LastInventoryItem;
#endif
extern int TrackCameraInit;
extern short TorchRoom;
extern int InitialiseGame;
extern int RequiredStartPos;
extern int WeaponDelay;
extern int WeaponEnemyTimer;
extern HEIGHT_TYPES HeightType;
extern int HeavyTriggered;
extern short SkyPos1;
extern short SkyPos2;
extern signed char SkyVelocity1;
extern signed char SkyVelocity2;
extern CVECTOR SkyColor1;
extern CVECTOR SkyColor2;
extern int CutSeqNum;
extern int CutSeqTriggered;
extern int GlobalPlayingCutscene;
extern int CurrentLevel;
extern bool SoundActive;
extern bool DoTheGame;
extern bool ThreadEnded;
extern int OnFloor;
extern int SmokeWindX;
extern int SmokeWindZ;
extern int OnObject;
extern int KillEverythingFlag;
extern int FlipTimer;
extern int FlipEffect;
extern int TriggerTimer;
extern int JustLoaded;
extern int PoisonFlags;
extern int OldLaraBusy;
extern int Infrared;
extern short FlashFadeR;
extern short FlashFadeG;
extern short FlashFadeB;
extern short FlashFader;
extern int TiltXOffset;
extern int TiltYOffset;
extern std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];
extern short IsRoomOutsideNo;
extern bool g_CollidedVolume;

GAME_STATUS DoTitle(int index);
GAME_STATUS DoLevel(int index, std::string ambient, bool loadFromSavegame);
GAME_STATUS ControlPhase(int numFrames, int demoMode);
void UpdateSky();
void AnimateWaterfalls();
short GetDoor(FLOOR_INFO* floor);
void TranslateItem(ITEM_INFO* item, int x, int y, int z);
void TestTriggers(short* data, int heavy, int HeavyFlags);
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
int TriggerActive(ITEM_INFO* item);
int GetWaterHeight(int x, int y, int z, short roomNumber);
int is_object_in_room(short roomNumber, short objectNumber);
void InterpolateAngle(short angle, short* rotation, short* outAngle, int shift);
int IsRoomOutside(int x, int y, int z);
void TestTriggersAtXYZ(int x, int y, int z, short roomNumber, int heavy, int flags);
void ResetGlobals();

unsigned CALLBACK GameMain(void*);