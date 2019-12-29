#pragma once
#include "..\Global\types.h"

struct NODEOFFSET_INFO
{
	short x; // size=0, offset=0
	short y; // size=0, offset=2
	short z; // size=0, offset=4
	char meshNum; // size=0, offset=6
	unsigned char gotIt; // size=0, offset=7
};

extern GUNFLASH_STRUCT Gunflashes[4]; // offset 0xA31D8
extern PHD_VECTOR NodeVectors[16]; // offset 0xA3274
extern FIRE_SPARKS FireSparks[20]; // offset 0xA94FC
extern SMOKE_SPARKS SmokeSparks[32]; // offset 0xA8F7C
extern GUNSHELL_STRUCT Gunshells[24]; // offset 0xA7DFC
extern BLOOD_STRUCT Blood[32]; // offset 0xA88FC
extern BUBBLE_STRUCT Bubbles[40]; // offset 0xA80FC
extern DRIP_STRUCT Drips[32]; // offset 0xA85FC
extern SHOCKWAVE_STRUCT ShockWaves[16]; // 0xA7C3C
extern FIRE_LIST Fires[32]; // offset 0xA8D7C

extern int NextFireSpark;
extern int NextSmokeSpark;
extern int NextBubble;
extern int NextDrip;
extern int NextBlood;
extern int NextSpider;
extern int NextGunShell;

void TriggerBlood(int x, int y, int z, int unk, int num);
void TriggerExplosionBubble(int x, int y, int z, short roomNum);
int GetFreeFireSpark();
void TriggerGlobalStaticFlame();
void TriggerGlobalFireSmoke();
void TriggerGlobalFireFlame();
void keep_those_fires_burning();
void ClearFires();
void AddFire(int x, int y, int z, char size, short roomNum, short on);
void UpdateFireSparks();
int GetFreeSmokeSpark();
void UpdateSmoke();
byte TriggerGunSmoke_SubFunction(int weaponType);
void TriggerGunSmoke(int x, int y, int z, short xv, short yv, short zv, byte initial, int weaponType, byte count);
void TriggerShatterSmoke(int x, int y, int z);
int GetFreeBlood();
void TriggerBlood(int x, int y, int z, int unk, int num);
void UpdateBlood();
int GetFreeGunshell();
void TriggerGunShell(short hand, short objNum, int weaponType);
void UpdateGunShells();
void AddWaterSparks(int x, int y, int z, int num);
int GetFreeBubble();
void CreateBubble(PHD_VECTOR* pos, short roomNum, int unk1, int unk2, int flags, int xv, int yv, int zv);
void LaraBubbles(ITEM_INFO* item);
void UpdateBubbles();
int GetFreeDrip();
void UpdateDrips();
void TriggerLaraDrips();
int ExplodingDeath2(short itemNumber, int meshBits, short damage);
int GetFreeShockwave();
void TriggerShockwave(PHD_3DPOS* pos, short innerRad, short outerRad, int speed, char r, char g, char b, char life, short angle, short flags);
void TriggerShockwaveHitEffect(int x, int y, int z, int color, short rot, int vel);
void UpdateShockwaves();
void TriggerSmallSplash(int x, int y, int z, int num);
int GetFreeSpider();
void SetFadeClip(short height, short speed);

void Inject_Tomb4FX();