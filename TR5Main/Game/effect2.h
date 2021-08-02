#pragma once
#include "phd_global.h"
#include "items.h"

enum RIPPLE_TYPE
{
	RIPPLE_FLAG_NONE = 0x0,
	RIPPLE_FLAG_SHORT_LIFE = 0x1,
	RIPPLE_FLAG_RAND_ROT = 0x20,
	RIPPLE_FLAG_RAND_POS = 0x40,
	RIPPLE_FLAG_BLOOD = 0x80,
	RIPPLE_FLAG_LOW_OPACITY = 0x02
};

enum SpriteEnumFlag
{
	SP_NONE = 0x0000,
	SP_FIRE = 0x0001,
	SP_SCALE = 0x0002,
	SP_BLOOD = 0x0004,
	SP_DEF = 0x0008,
	SP_ROTATE = 0x0010,
	SP_EXPLOSION = 0x0020,
	SP_FX = 0x0040,
	SP_ITEM = 0x0080,
	SP_WIND = 0x0100,
	SP_EXPDEF = 0x0200,
	SP_DAMAGE = 0x0400,
	SP_UNDERWEXP = 0x0800,
	SP_NODEATTACH = 0x1000,
	SP_PLASMAEXP = 0x2000
};

enum TransTypeEnum
{
	NOTRANS,
	SEMITRANS,
	COLADD,
	COLSUB,
	WEIRD
};

enum FireSizeEnum
{
	SP_TINYFIRE,
	SP_SMALLFIRE,
	SP_BIGFIRE
};

struct NODEOFFSET_INFO
{
	short x;
	short y;
	short z;
	char meshNum;
	unsigned char gotIt;
};

struct SPLASH_SETUP
{
	float x;
	float y;
	float z;
	float splashPower;
	float innerRadius;
	int room;
};

struct RIPPLE_STRUCT
{
	Vector4 currentColor;
	Vector4 initialColor;
	Vector3 worldPos;
	unsigned int SpriteID;
	float rotation;
	float size;
	float sizeRate;
	float life; //max life
	float lifeTime; // current life
	float lifeRate; // life change rate
	bool active;
	bool isBillboard; //used for Blood
};

struct SPARKS
{
	int x;
	int y;
	int z;
	short xVel;
	short yVel;
	short zVel;
	short gravity;
	short rotAng;
	unsigned short flags; // SP_enum
	float sSize;
	float dSize;
	float size;
	unsigned char friction;
	unsigned char scalar;
	unsigned char def;
	signed char rotAdd;
	signed char maxYvel;
	bool on;
	byte sR;
	byte sG;
	byte sB;
	byte dR;
	byte dG;
	byte dB;
	byte r;
	byte g;
	byte b;
	unsigned char colFadeSpeed;
	unsigned char fadeToBlack;
	unsigned char sLife;
	unsigned char life;
	TransTypeEnum transType;
	unsigned char extras;
	signed char dynamic;
	unsigned char fxObj;
	unsigned char roomNumber;
	unsigned char nodeNumber;
};

struct SPLASH_STRUCT
{
	float x;
	float y;
	float z;
	float innerRad;
	float innerRadVel;
	float heightVel;
	float heightSpeed;
	float height;
	float outerRad;
	float outerRadVel;
	float animationSpeed;
	float animationPhase;
	short spriteSequenceStart;
	short spriteSequenceEnd;
	unsigned short life;
	bool isRipple;
	bool isActive;
};

struct DYNAMIC
{
	int x;
	int y;
	int z;
	byte on;
	byte r;
	byte g;
	byte b;
	short falloff;
	byte used;
	byte pad1[1];
	int FalloffScale;
};

struct SP_DYNAMIC
{
	byte On;
	byte Falloff;
	byte R;
	byte G;
	byte B;
	byte Flags;
	byte Pad[2];
};

constexpr auto SD_EXPLOSION = 1;
constexpr auto SD_UWEXPLOSION = 2;

#define MAX_NODE 23
#define MAX_DYNAMICS 64
#define MAX_SPARKS 1024
#define MAX_RIPPLES 256
#define MAX_SPLASHES 8
#define MAX_SPARKS_DYNAMICS 8

extern int NextSpark;
extern int DeadlyBounds[6];
extern SPLASH_SETUP SplashSetup;
extern SPLASH_STRUCT Splashes[MAX_SPLASHES];
extern RIPPLE_STRUCT Ripples[MAX_RIPPLES];
extern DYNAMIC Dynamics[MAX_DYNAMICS];
extern SPARKS Sparks[MAX_SPARKS];
extern SP_DYNAMIC SparkDynamics[MAX_SPARKS_DYNAMICS];
extern int SmokeWeapon;
extern byte SmokeCountL;
extern byte SmokeCountR;
extern int SplashCount;
extern PHD_VECTOR NodeVectors[MAX_NODE];
extern NODEOFFSET_INFO NodeOffsets[MAX_NODE];

void DetatchSpark(int num, SpriteEnumFlag type);
int GetFreeSpark();
void UpdateSparks();
void TriggerRicochetSpark(GAME_VECTOR* pos, short angle, int num, int unk);
void TriggerCyborgSpark(int x, int y, int z, short xv, short yv, short zv);
void TriggerExplosionSparks(int x, int y, int z, int extraTrig, int dynamic, int uw, int roomNumber);
void TriggerExplosionSmokeEnd(int x, int y, int z, int uw);
void TriggerExplosionSmoke(int x, int y, int z, int uw);
void TriggerFireFlame(int x, int y, int z, int fxObj, int type);
void TriggerSuperJetFlame(ITEM_INFO* item, int yvel, int deadly);
void SetupSplash(const SPLASH_SETUP* const setup,int room);
void UpdateSplashes();
void SetupRipple(int x, int y, int z, float size, char flags,unsigned int spriteID,float rotation = 0);
void TriggerUnderwaterBlood(int x, int y, int z, int sizeme);
void TriggerWaterfallMist(int x, int y, int z, int angle);
void TriggerDartSmoke(int x, int y, int z, int xv, int zv, int hit);
void KillAllCurrentItems(short itemNumber);
void TriggerDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b);
void ClearDynamicLights();
void TriggerRocketFlame(int x, int y, int z, int xv, int yv, int zv, int itemNumber);
void TriggerRocketSmoke(int x, int y, int z, int bodyPart);
void TriggerFireFlame(int x, int y, int z, int flag1, int flag2);
void TriggerFlashSmoke(int x, int y, int z, short roomNumber);
void TriggerMetalSparks(int x, int y, int z, int xv, int yv, int zv, int additional);
void WadeSplash(ITEM_INFO* item, int wh, int wd);
void Splash(ITEM_INFO* item);
void TriggerRocketFire(int x, int y, int z);
void TriggerExplosionBubbles(int x, int y, int z, short roomNumber);