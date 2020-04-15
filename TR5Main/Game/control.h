#pragma once

#include "..\Global\global.h"

#define DoRayBox_sub_401523 ((int (__cdecl*)(PHD_VECTOR*, PHD_VECTOR*, PHD_VECTOR*, PHD_VECTOR*, PHD_VECTOR*)) 0x00401523)

#define TRIG_BITS(T) ((T & 0x3fff) >> 10)

extern int KeyTriggerActive;
extern byte IsAtmospherePlaying;
extern byte FlipStatus;
extern int FlipStats[255];
extern int FlipMap[255];
extern bool InItemControlLoop;
extern short ItemNewRoomNo;
extern short ItemNewRooms[512];
extern short NextFxActive;
extern short NextFxFree;
extern short NextItemActive;
extern short NextItemFree;
extern short* TriggerIndex;
extern int DisableLaraControl;

GAME_STATUS DoTitle(int index);
GAME_STATUS DoLevel(int index, int ambient, bool loadFromSavegame);
GAME_STATUS ControlPhase(int numFrames, int demoMode);
void UpdateSky();
void AnimateWaterfalls();
void ActivateKey();
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
void SeedRandomControl(int seed);
int GetRandomDraw();
void SeedRandomDraw(int seed);
int GetCeiling(FLOOR_INFO* floor, int x, int y, int z);
int DoRayBox(GAME_VECTOR* start, GAME_VECTOR* end, short* box, PHD_3DPOS* itemOrStaticPos, PHD_VECTOR* hitPos, short closesItemNumber);
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

unsigned __stdcall GameMain(void*);
void Inject_Control();
