#pragma once
#include "Game/effects/effects.h"
#include "Game/Lara/lara_struct.h"
#include "Specific/phd_global.h"
#include "Renderer/Renderer11Enums.h"

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
	short rotAng;
	short flags;
	byte sSize;
	byte dSize;
	byte size;
	byte friction;
	byte scalar;
	byte def;
	signed char rotAdd;
	signed char maxYvel;
	byte on;
	byte sShade;
	byte dShade;
	byte shade;
	byte colFadeSpeed;
	byte fadeToBlack;
	signed char sLife;
	signed char life;
	BLEND_MODES blendMode;
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
	short xRot;
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
	PHD_3DPOS pos;
	short fallspeed;
	short roomNumber;
	short speed;
	short counter;
	short dirXrot;
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
	float size;
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
	short rotAng;
	short flags;
	unsigned char sSize;
	unsigned char dSize;
	unsigned char size;
	unsigned char friction;
	unsigned char scalar;
	unsigned char def;
	signed char rotAdd;
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
	short rotAng;
	unsigned char sSize;
	unsigned char dSize;
	unsigned char size;
	unsigned char friction;
	byte rotAdd;
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

constexpr auto MAX_SPARKS_FIRE = 20;
constexpr auto MAX_FIRE_LIST = 32;
constexpr auto MAX_SPARKS_SMOKE = 32;
constexpr auto MAX_SPARKS_BLOOD = 32;
constexpr auto MAX_GUNFLASH = 4;
constexpr auto MAX_GUNSHELL = 24;
constexpr auto MAX_DRIPS = 32;
constexpr auto MAX_SHOCKWAVE = 16;

constexpr auto BODY_NO_BOUNCE     = 0x0001;
constexpr auto BODY_GIBS		  = 0x0002;
constexpr auto BODY_EXPLODE		  = 0x0100;
constexpr auto BODY_NO_BOUNCE_ALT = 0x0200;
constexpr auto BODY_STONE_SOUND   = 0x0800;

extern GUNFLASH_STRUCT Gunflashes[MAX_GUNFLASH];
extern FIRE_SPARKS FireSparks[MAX_SPARKS_FIRE];
extern SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE];
extern GUNSHELL_STRUCT Gunshells[MAX_GUNSHELL];
extern BLOOD_STRUCT Blood[MAX_SPARKS_BLOOD];
extern DRIP_STRUCT Drips[MAX_DRIPS];
extern SHOCKWAVE_STRUCT ShockWaves[MAX_SHOCKWAVE];
extern FIRE_LIST Fires[MAX_FIRE_LIST];

void TriggerBlood(int x, int y, int z, int unk, int num);
void TriggerExplosionBubble(int x, int y, int z, short roomNumber);
int GetFreeFireSpark();
void TriggerGlobalStaticFlame();
void TriggerGlobalFireSmoke();
void TriggerGlobalFireFlame();
void TriggerPilotFlame(int itemNum, int nodeIndex);
void ThrowFire(int itemNum, int meshIndex, Vector3Int offset, Vector3Int speed);
void ThrowPoison(int itemNum, int meshIndex, Vector3Int offset, Vector3Int speed, Vector3 color);
void UpdateFireProgress();
void ClearFires();
void AddFire(int x, int y, int z, short roomNum, float size, short fade);
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
void ExplodingDeath(short itemNumber, short flags); // EXPLODE_ flags
int GetFreeShockwave();
void TriggerShockwave(PHD_3DPOS* pos, short innerRad, short outerRad, int speed, unsigned char r, unsigned char g, unsigned char b, unsigned char life, short angle, short flags);
void TriggerShockwaveHitEffect(int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, short rot, int vel);
void UpdateShockwaves();
void TriggerSmallSplash(int x, int y, int z, int number);
void TriggerUnderwaterExplosion(ItemInfo* item, int flag);
void ExplodeVehicle(ItemInfo* laraItem, ItemInfo* vehicle);