#pragma once
#include "..\Global\types.h"
#include "..\Global\constants.h"

struct ENERGY_ARC
{
	PHD_VECTOR pos1;	// 0
	PHD_VECTOR pos2;	// 12
	PHD_VECTOR pos3;	// 24
	PHD_VECTOR pos4;	// 36
	byte r;				// 48
	byte g;				// 49
	byte b;				// 50
	byte life;			// 51
	byte byte34;		// 52
	byte gap35[8];		// 53
	byte byte3D;		// 61
	byte flags;			// 62
	byte byte3F;		// 63
	int segmentSize;		// 64
};

extern int LaserSightX;
extern int LaserSightY;
extern int LaserSightZ;
extern char LaserSightActive;
extern char LaserSightCol;

extern int NextFireSpark;
extern int NextSmokeSpark;
extern int NextBubble;
extern int NextDrip;
extern int NextBlood;
extern int NextSpider;
extern int NextGunShell;

extern SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE];
extern ENERGY_ARC EnergyArcs[MAX_ENERGY_ARCS];

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
void SetFadeClip(short height, short speed);
void TriggerLightningGlow(int x, int y, int z, int rgb);
void TriggerEnergyArc(PHD_VECTOR* start, PHD_VECTOR* end, byte r, byte g, byte b, int  segmentSize, byte life);
void UpdateEnergyArcs();

void Inject_Tomb4FX();