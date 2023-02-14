#pragma once
#include "Game/effects/effects.h"
#include "Game/Lara/lara_struct.h"
#include "Math/Math.h"
#include "Renderer/Renderer11Enums.h"

enum class LaraWeaponType;
struct ItemInfo;

enum BodyPartFlags
{
	BODY_NO_BOUNCE	   = (1 << 0),
	BODY_GIBS		   = (1 << 1),
	BODY_EXPLODE	   = (1 << 8),
	BODY_NO_BOUNCE_ALT = (1 << 9),
	BODY_STONE_SOUND   = (1 << 11)
};

struct Matrix3D
{
	short m00, m01, m02;
	short m10, m11, m12;
	short m20, m21, m22;
	short pad;
	int tx, ty, tz;
};

struct SMOKE_SPARKS
{
	int x;
	int y;
	int z;
	int xVel;
	int yVel;
	int zVel;
	int gravity;
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
	Matrix3D matrix;
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
	short yRot;
	short zRot;
	short damage;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char sr;
	unsigned char sg;
	unsigned char sb;
	unsigned char life;
	short speed;
	short temp;
	bool fadeIn = false;
	int style;
};

struct GUNSHELL_STRUCT
{
	Pose pos;
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

enum class ShockwaveStyle
{
	Normal = 0,
	Sophia = 1
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
extern int NextBlood;
extern int NextSpider;
extern int NextGunShell;

constexpr auto MAX_SPARKS_FIRE = 20;
constexpr auto MAX_FIRE_LIST = 32;
constexpr auto MAX_SPARKS_SMOKE = 32;
constexpr auto MAX_SPARKS_BLOOD = 32;
constexpr auto MAX_GUNFLASH = 4;
constexpr auto MAX_GUNSHELL = 24;
constexpr auto MAX_SHOCKWAVE = 16;

extern GUNFLASH_STRUCT Gunflashes[MAX_GUNFLASH];
extern FIRE_SPARKS FireSparks[MAX_SPARKS_FIRE];
extern SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE];
extern GUNSHELL_STRUCT Gunshells[MAX_GUNSHELL];
extern BLOOD_STRUCT Blood[MAX_SPARKS_BLOOD];
extern SHOCKWAVE_STRUCT ShockWaves[MAX_SHOCKWAVE];
extern FIRE_LIST Fires[MAX_FIRE_LIST];

void TriggerBlood(int x, int y, int z, int unk, int num);
void TriggerExplosionBubble(int x, int y, int z, short roomNumber);
int GetFreeFireSpark();
void TriggerGlobalStaticFlame();
void TriggerGlobalFireSmoke();
void TriggerGlobalFireFlame();
void TriggerPilotFlame(int itemNum, int nodeIndex);
void ThrowFire(int itemNum, int meshIndex, Vector3i offset, Vector3i speed);
void ThrowPoison(int itemNum, int meshIndex, Vector3i offset, Vector3i speed, Vector3 color);
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
void ExplodingDeath(short itemNumber, short flags); // EXPLODE_ flags
int GetFreeShockwave();
void TriggerShockwave(Pose* pos, short innerRad, short outerRad, int speed, unsigned char r, unsigned char g, unsigned char b, unsigned char life, EulerAngles rotation, short damage, bool sound, bool fadein, int style);
void TriggerShockwaveHitEffect(int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, short rot, int vel);
void UpdateShockwaves();
void TriggerSmallSplash(int x, int y, int z, int number);
void TriggerUnderwaterExplosion(ItemInfo* item, int flag);
void ExplodeVehicle(ItemInfo* laraItem, ItemInfo* vehicle);