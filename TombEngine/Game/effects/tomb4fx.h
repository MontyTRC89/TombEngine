#pragma once
#include "Game/effects/effects.h"
#include "Game/Lara/lara_struct.h"
#include "Math/Math.h"
#include "Renderer/RendererEnums.h"

enum class LaraWeaponType;
struct CreatureBiteInfo;
struct ItemInfo;

enum BodyPartFlags
{
	BODY_NO_BOUNCE			  = (1 << 0), // No bounce.
	BODY_GIBS				  = (1 << 1), // Add blood and SFX_TR4_LARA_THUD upon floor collision.
	BODY_PART_EXPLODE		  = (1 << 2), // Explode upon impact. Requires BODY_EXPLODE flag.
	BODY_NO_FLAME			  = (1 << 3), // No flame.
	BODY_NO_RAND_VELOCITY	  = (1 << 4), // No random velocity.
	BODY_MORE_RAND_VELOCITY	  = (1 << 5), // Add more randomness to velocity.
	BODY_NO_VERTICAL_VELOCITY = (1 << 6), // No vertical velocity.
	BODY_LESS_IMPULSE		  = (1 << 7), // Add less vertical velocity than normal. TODO: Weird name.
	BODY_DO_EXPLOSION		  = (1 << 8), // Explode.
	BODY_NO_BOUNCE_ALT		  = (1 << 9), // Same as BODY_NO_BOUNCE, but with delay before despawning.
	BODY_STONE_SOUND		  = (1 << 11), // Do impact sound if stone. NOTE: BODY_GIBS also add sound, but this one is prioritary.
	BODY_NO_SMOKE			  = (1 << 12),  // Remove smoke upon despawn or shatter.
	BODY_NO_SHATTER_EFFECT	  = (1 << 13), // Remove shatter effect upon despawn.
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
	BlendMode blendMode;
	byte fxObj;
	byte nodeNumber;
	byte mirror;
};

struct SHOCKWAVE_STRUCT
{
	int x;
	int y;
	int z;
	short xRot;
	short yRot;
	short zRot;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char sr;
	unsigned char sg;
	unsigned char sb;

	short innerRad;
	short outerRad;

	unsigned char life;
	short sLife;
	short speed;
	short damage;
	int style;

	bool fadeIn = false;
	bool HasLight = false;
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

enum class ShockwaveStyle
{
	Normal = 0,
	Sophia = 1,
	Knockback = 2,
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
extern int NextSpider;
extern int NextGunShell;

constexpr auto MAX_SPARKS_FIRE = 20;
constexpr auto MAX_FIRE_LIST = 32;
constexpr auto MAX_SPARKS_SMOKE = 32;
constexpr auto MAX_GUNFLASH = 4;
constexpr auto MAX_GUNSHELL = 24;
constexpr auto MAX_SHOCKWAVE = 16;

extern FIRE_SPARKS FireSparks[MAX_SPARKS_FIRE];
extern SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE];
extern GUNSHELL_STRUCT Gunshells[MAX_GUNSHELL];
extern SHOCKWAVE_STRUCT ShockWaves[MAX_SHOCKWAVE];
extern FIRE_LIST Fires[MAX_FIRE_LIST];

void TriggerExplosionBubble(int x, int y, int z, short roomNumber);
int GetFreeFireSpark();
void TriggerGlobalStaticFlame();
void TriggerGlobalFireSmoke();
void TriggerGlobalFireFlame();
void TriggerPilotFlame(int itemNumber, int nodeIndex);
void ThrowFire(int itemNumber, int meshID, const Vector3i& offset, const Vector3i& vel, int spriteID = 0);
void ThrowFire(int itemNumber, const CreatureBiteInfo& bite, const Vector3i& vel, int spriteID = 0);
void ThrowPoison(const ItemInfo& item, int boneID, const Vector3& offset, const Vector3& vel, const Color& colorStart, const Color& colorEnd, int spriteID = 0);
void ThrowPoison(const ItemInfo& item, const CreatureBiteInfo& bite, const Vector3& vel, const Color& colorStart, const Color& colorEnd, int spriteID = 0);
void UpdateFireProgress();
void ClearFires();
void AddFire(int x, int y, int z, short roomNum, float size, short fade);
void UpdateFireSparks();
int GetFreeSmokeSpark();
void UpdateSmoke();
byte TriggerGunSmoke_SubFunction(LaraWeaponType weaponType);
void TriggerGunSmoke(int x, int y, int z, short xv, short yv, short zv, byte initial, LaraWeaponType weaponType, byte count);
void TriggerShatterSmoke(int x, int y, int z);
int GetFreeGunshell();
void TriggerGunShell(short hand, short objNum, LaraWeaponType weaponType);
void UpdateGunShells();
void AddWaterSparks(int x, int y, int z, int num);
void ExplodingDeath(short itemNumber, short flags); // BODY_ flags
int GetFreeShockwave();
void TriggerShockwave(Pose* pos, short innerRad, short outerRad, int speed, unsigned char r, unsigned char g, unsigned char b, unsigned char life, EulerAngles rotation, short damage, bool sound, bool fadein, bool light, int style);
void TriggerShockwaveHitEffect(int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, short rot, int vel);
void UpdateShockwaves();
void TriggerSmallSplash(int x, int y, int z, int number);
void TriggerUnderwaterExplosion(ItemInfo* item, int flag);
void ExplodeVehicle(ItemInfo* laraItem, ItemInfo* vehicle);