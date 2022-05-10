#pragma once
#include "Game/effects/effects.h"
#include "Game/Lara/lara_struct.h"
#include "Specific/phd_global.h"

enum class LaraWeaponType;
struct ItemInfo;

struct SMOKE_SPARKS
{
	int x;
	int y;
	int z;
	short xVel;
	short yVel;
	short zVel;
	short gravity;
	float rotAng;
	short flags;
	byte sSize;
	byte dSize;
	byte size;
	byte friction;
	byte scalar;
	byte def;
	float rotAdd;
	signed char maxYvel;
	byte on;
	byte sShade;
	byte dShade;
	byte shade;
	byte colFadeSpeed;
	byte fadeToBlack;
	signed char sLife;
	signed char life;
	TransTypeEnum transType;
	byte fxObj;
	byte nodeNumber;
	byte mirror;
};

struct GUNFLASH_STRUCT
{
	MATRIX3D matrix;
	short on;
};

struct SHOCKWAVE_STRUCT
{
	int x;
	int y;
	int z;
	short innerRad;
	short outerRad;
	float xRot;
	short flags;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char life;
	short speed;
	short temp;
};

struct GUNSHELL_STRUCT
{
	PoseData pos;
	short fallspeed;
	short roomNumber;
	short speed;
	short counter;
	float dirXrot;
	short objectNumber;
};

struct DRIP_STRUCT
{
	int x;
	int y;
	int z;
	byte on;
	byte r;
	byte g;
	byte b;
	short yVel;
	byte gravity;
	byte life;
	short roomNumber;
	byte outside;
	byte pad;
};

struct FIRE_LIST
{
	int x;
	int y;
	int z;
	byte on;
	byte size;
	short roomNumber;
};

struct FIRE_SPARKS
{
	short x;
	short y;
	short z;
	short xVel;
	short yVel;
	short zVel;
	short gravity;
	float rotAng;
	short flags;
	unsigned char sSize;
	unsigned char dSize;
	unsigned char size;
	unsigned char friction;
	unsigned char scalar;
	unsigned char def;
	float rotAdd;
	signed char maxYvel;
	unsigned char on;
	unsigned char sR;
	unsigned char sG;
	unsigned char sB;
	unsigned char dR;
	unsigned char dG;
	unsigned char dB;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char colFadeSpeed;
	unsigned char fadeToBlack;
	unsigned char sLife;
	unsigned char life;
};

struct BLOOD_STRUCT
{
	int x;
	int y;
	int z;
	short xVel;
	short yVel;
	short zVel;
	short gravity;
	float rotAng;
	unsigned char sSize;
	unsigned char dSize;
	unsigned char size;
	unsigned char friction;
	float rotAdd;
	unsigned char on;
	unsigned char sShade;
	unsigned char dShade;
	unsigned char shade;
	unsigned char colFadeSpeed;
	unsigned char fadeToBlack;
	byte sLife;
	byte life;
	byte pad;
};

#define ENERGY_ARC_STRAIGHT_LINE	0
#define ENERGY_ARC_CIRCLE			1
#define ENERGY_ARC_NO_RANDOMIZE		1

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

#define MAX_SPARKS_FIRE 20
#define MAX_FIRE_LIST 32
#define MAX_SPARKS_SMOKE 32
#define MAX_SPARKS_BLOOD 32
#define MAX_GUNFLASH 4
#define MAX_GUNSHELL 24
#define MAX_DRIPS 32
#define MAX_SHOCKWAVE 16

extern GUNFLASH_STRUCT Gunflashes[MAX_GUNFLASH];
extern FIRE_SPARKS FireSparks[MAX_SPARKS_FIRE];
extern SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE];
extern GUNSHELL_STRUCT Gunshells[MAX_GUNSHELL];
extern BLOOD_STRUCT Blood[MAX_SPARKS_BLOOD];
extern DRIP_STRUCT Drips[MAX_DRIPS];
extern SHOCKWAVE_STRUCT ShockWaves[MAX_SHOCKWAVE];
extern FIRE_LIST Fires[MAX_FIRE_LIST];
extern SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE];

void TriggerBlood(int x, int y, int z, int unk, int num);
void TriggerExplosionBubble(int x, int y, int z, short roomNumber);
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
byte TriggerGunSmoke_SubFunction(LaraWeaponType weaponType);
void TriggerGunSmoke(int x, int y, int z, short xv, short yv, short zv, byte initial, LaraWeaponType weaponType, byte count);
void TriggerShatterSmoke(int x, int y, int z);
int GetFreeBlood();
void TriggerBlood(int x, int y, int z, int unk, int num);
void UpdateBlood();
int GetFreeGunshell();
void TriggerGunShell(short hand, short objNum, LaraWeaponType weaponType);
void UpdateGunShells();
void AddWaterSparks(int x, int y, int z, int num);
int GetFreeBubble();
void LaraBubbles(ItemInfo* item);
void DisableBubbles();
void UpdateBubbles();
int GetFreeDrip();
void UpdateDrips();
void TriggerLaraDrips(ItemInfo* item);

constexpr auto EXPLODE_HIT_EFFECT = 258;
constexpr auto EXPLODE_NORMAL = 256;
int ExplodingDeath(short itemNumber, int meshBits, short flags); // EXPLODE_ flags

int GetFreeShockwave();
void TriggerShockwave(PoseData* pos, short innerRad, short outerRad, int speed, char r, char g, char b, char life, float angle, short flags);
void TriggerShockwaveHitEffect(int x, int y, int z, int color, short rot, int vel);
void UpdateShockwaves();
void TriggerSmallSplash(int x, int y, int z, int number);
void SetFadeClip(short height, int velocity);
