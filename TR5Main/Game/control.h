#pragma once

#include "..\Global\global.h"

#define GetCeiling ((int (__cdecl*)(FLOOR_INFO*, int, int, int)) 0x00417640)
#define GetRandomControl ((int (__cdecl*)()) 0x004A7C10)
#define AnimateItem ((void (__cdecl*)(ITEM_INFO*)) 0x00415300)
#define GetWaterHeight ((int (__cdecl*)(int, int, int, short)) 0x00415DA0)
#define TriggerActive ((int (__cdecl*)(ITEM_INFO*)) 0x004175B0)
//#define GetChange ((int (__cdecl*)(ITEM_INFO*, ANIM_STRUCT*)) 0x00415890)
//#define KillMoveItems ((void (__cdecl*)()) 0x00414620)
//#define KillMoveEffects ((void (__cdecl*)()) 0x004146A0)
#define ClearDynamics ((void (__cdecl*)()) 0x00431530)
#define ClearFires ((void (__cdecl*)()) 0x00481B10)
#define UpdateSparks ((void (__cdecl*)()) 0x0042E8B0)	
#define UpdateFireSparks ((void (__cdecl*)()) 0x004813B0)	
#define UpdateSmoke ((void (__cdecl*)()) 0x00481DD0)	
#define UpdateBlood ((void (__cdecl*)()) 0x00482610)	
#define UpdateBubbles ((void (__cdecl*)()) 0x00483540)
#define UpdateSplashes ((void (__cdecl*)()) 0x00430710)
#define UpdateDebris ((void (__cdecl*)()) 0x0041D500)
#define UpdateDrips ((void (__cdecl*)()) 0x00483D90)
#define UpdateGunShells ((void (__cdecl*)()) 0x00482D80)
#define UpdateBats ((void (__cdecl*)()) 0x00407FD0)
#define UpdateRats ((void (__cdecl*)()) 0x0046AC70)
#define UpdateSpiders ((void (__cdecl*)()) 0x0047A340)
#define UpdateShockwaves ((void (__cdecl*)()) 0x004849A0)
#define UpdateLightning ((void (__cdecl*)()) 0x00484CB0)
#define UpdatePulseColor ((void (__cdecl*)()) 0x00480830)
#define RumbleScreen ((void (__cdecl*)()) 0x00442C90)
#define ExplodeItemNode ((int (__cdecl*)(ITEM_INFO*, int, int, int)) 0x0041ABF0)
#define LavaBurn ((void (__cdecl*)(ITEM_INFO*)) 0x0048ADD0)
#define DoFlipMap ((void (__cdecl*)(short)) 0x00418910)
#define PlaySoundTrack ((void (__cdecl*)(short, short)) 0x00418B90)
//#define AlterFloorHeight ((void (__cdecl*)(ITEM_INFO*, int)) 0x004159F0)
#define SoundEffects ((void (__cdecl*)()) 0x00432640)
#define ObjectOnLOS2 ((int (__cdecl*)(GAME_VECTOR*, GAME_VECTOR*, PHD_VECTOR*, BITE_INFO*)) 0x00419110)
#define LOS ((int (__cdecl*)(GAME_VECTOR*, GAME_VECTOR*)) 0x00417CF0)
#define GetTargetOnLOS ((int (__cdecl*)(GAME_VECTOR*, GAME_VECTOR*, int, int)) 0x0041A170)

//#define GetFloor ((FLOOR_INFO* (__cdecl*)(int, int, int, short*)) 0x00415B20)
//#define GetFloorHeight ((int (__cdecl*)(FLOOR_INFO*, int, int, int)) 0x00415FB0)

#define TRIG_BITS(T) ((T & 0x3fff) >> 10)

GAME_STATUS DoTitle(int index);
GAME_STATUS DoLevel(int index, int ambient, bool loadFromSavegame);
GAME_STATUS ControlPhase(int numFrames, int demoMode);
void UpdateSky();
void AnimateWaterfalls();
void ActivateKey();
short GetDoor(FLOOR_INFO* floor);
void TranslateItem(ITEM_INFO* item, int x, int y, int z);
void TestTriggers(short* data, int heavy, int HeavyFlags);
int CheckNoColFloorTriangle(FLOOR_INFO* floor, short x, short z);
int GetWaterSurface(int x, int y, int z, short roomNumber);
void KillMoveItems();
void KillMoveEffects();
int GetChange(ITEM_INFO* item, ANIM_STRUCT* anim);
void AlterFloorHeight(ITEM_INFO* item, int height);
int CheckNoColCeilingTriangle(FLOOR_INFO* floor, int x, int z);
int CheckNoColFloorTriangle(FLOOR_INFO* floor, int x, int z);
int GetFloorHeight(FLOOR_INFO* floor, int x, int y, int z);
FLOOR_INFO* GetFloor(int x, int y, int z, short* roomNumber);

unsigned __stdcall GameMain(void*);
void Inject_Control();
