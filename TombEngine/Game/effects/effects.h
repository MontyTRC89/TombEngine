#pragma once
#include "Math/Math.h"
#include "Renderer/Renderer11Enums.h"

enum class LaraWeaponType;
enum GAME_OBJECT_ID : short;
struct CollisionInfo;
struct ItemInfo;

constexpr auto SD_EXPLOSION	  = 1;
constexpr auto SD_UWEXPLOSION = 2;

constexpr auto MAX_NODE		= 23;
constexpr auto MAX_DYNAMICS = 64;
constexpr auto MAX_SPLASHES = 8;
constexpr auto NUM_EFFECTS	= 256;

constexpr auto MAX_PARTICLES		 = 1024;
constexpr auto MAX_PARTICLE_DYNAMICS = 8;

enum SpriteEnumFlag
{
	SP_NONE		  = 0,
	SP_FIRE		  = (1 << 0),
	SP_SCALE	  = (1 << 1),
	SP_BLOOD	  = (1 << 2),
	SP_DEF		  = (1 << 3),
	SP_ROTATE	  = (1 << 4),
	SP_EXPLOSION  = (1 << 5),
	SP_FX		  = (1 << 6),
	SP_ITEM		  = (1 << 7),
	SP_WIND		  = (1 << 8),
	SP_EXPDEF	  = (1 << 9),
	SP_DAMAGE	  = (1 << 10),
	SP_UNDERWEXP  = (1 << 11),
	SP_NODEATTACH = (1 << 12),
	SP_PLASMAEXP  = (1 << 13),
	SP_POISON	  = (1 << 14),
	SP_COLOR	  = (1 << 15),
};

// Used by Particle.nodeNumber.
enum ParticleNodeOffsetIDs
{
	NodeUnknown0, // TR5
	NodeUnknown1,
	NodeUnknown2,
	NodeUnknown3,
	NodeUnknown4,
	NodeUnknown5,
	NodeUnknown6,
	NodeUnknown7,
	NodeUnknown8,
	NodePilotFlame, // TR3-5
	NodeWasp,
	NodePunkFlame,
	NodePendulumFlame,
	NodeTonyHandLeftFlame,
	NodeTonyHandRightFlame,
	NodeClawMutantPlasma,
	NodeWillardBossLeftPlasma,
	NodeWillardBossRightPlasma,
	NodeEmpty, // Empty node (mesh 0, position 0)
	NodeMax
};

enum class FlameType
{
	Big,
	Medium,
	Small,
	Static,
	Pulse,
	SmallFast,
	Trail
};

struct FX_INFO
{
	Pose pos;
	short roomNumber;
	short objectNumber;
	short nextFx;
	short nextActive;
	short speed;
	short fallspeed;
	int frameNumber;
	short counter;
	Vector4 color;
	short flag1;
	short flag2;
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
	int x;
	int y;
	int z;
	char flags;
	unsigned char life;
	unsigned char size;
	unsigned char init;
};

struct Particle
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
	unsigned char spriteIndex;
	signed char rotAdd;
	signed char maxYvel;
	bool on;
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
	int sLife;
	int life;
	BLEND_MODES blendMode;
	unsigned char extras;
	signed char dynamic;
	unsigned char fxObj;
	unsigned char roomNumber;
	unsigned char nodeNumber; // ParticleNodeOffsetIDs enum.
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

struct ParticleDynamic
{
	byte On;
	byte Falloff;
	byte R;
	byte G;
	byte B;
	byte Flags;
	byte Pad[2];
};

extern GameBoundingBox DeadlyBounds;

// New particle class
extern Particle Particles[MAX_PARTICLES];
extern ParticleDynamic ParticleDynamics[MAX_PARTICLE_DYNAMICS];

extern SPLASH_SETUP SplashSetup;
extern SPLASH_STRUCT Splashes[MAX_SPLASHES];

extern Vector3i NodeVectors[ParticleNodeOffsetIDs::NodeMax];
extern NODEOFFSET_INFO NodeOffsets[ParticleNodeOffsetIDs::NodeMax];

extern FX_INFO EffectList[NUM_EFFECTS];

template <typename TEffect>
TEffect& GetNewEffect(std::vector<TEffect>& effects, unsigned int countMax)
{
	// Add and return new effect.
	if (effects.size() < countMax)
		return effects.emplace_back();

	TEffect* effectPtr = nullptr;
	float shortestLife = INFINITY;

	// Find effect with shortest remaining life.
	for (auto& effect : effects)
	{
		if (effect.Life < shortestLife)
		{
			effectPtr = &effect;
			shortestLife = effect.Life;
		}
	}

	// Clear and return existing effect.
	*effectPtr = {};
	return *effectPtr;
}

template <typename TEffect>
void ClearInactiveEffects(std::vector<TEffect>& effects)
{
	effects.erase(
		std::remove_if(
			effects.begin(), effects.end(),
			[](const TEffect& effect) { return (effect.Life <= 0.0f); }),
		effects.end());
}

Particle* GetFreeParticle();

void SetSpriteSequence(Particle& particle, GAME_OBJECT_ID objectID);

void DetatchSpark(int num, SpriteEnumFlag type);
void UpdateSparks();
void TriggerRicochetSpark(const GameVector& pos, short angle, int count, int unk);
void TriggerCyborgSpark(int x, int y, int z, short xv, short yv, short zv);
void TriggerExplosionSparks(int x, int y, int z, int extraTrig, int dynamic, int uw, int roomNumber, const Vector3& mainColor = Vector3::Zero, const Vector3& secondColor = Vector3::Zero);
void TriggerExplosionSmokeEnd(int x, int y, int z, int uw);
void TriggerExplosionSmoke(int x, int y, int z, int uw);
void TriggerFireFlame(int x, int y, int z, FlameType type, const Vector3& color1 = Vector3::Zero, const Vector3& color2 = Vector3::Zero);
void TriggerSuperJetFlame(ItemInfo* item, int yvel, int deadly);
void SetupSplash(const SPLASH_SETUP* const setup, int room);
void UpdateSplashes();
void TriggerLaraBlood();
short DoBloodSplat(int x, int y, int z, short speed, short yRot, short roomNumber);
void DoLotsOfBlood(int x, int y, int z, int speed, short direction, short roomNumber, int count);
void ControlWaterfallMist(short itemNumber);
void TriggerWaterfallMist(const ItemInfo& item);
void KillAllCurrentItems(short itemNumber);
void TriggerDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b);
void TriggerRocketFlame(int x, int y, int z, int xv, int yv, int zv, int itemNumber);
void TriggerRocketSmoke(int x, int y, int z, int bodyPart);
void TriggerFlashSmoke(int x, int y, int z, short roomNumber);
void TriggerMetalSparks(int x, int y, int z, int xv, int yv, int zv, const Vector3& color, int additional);
void TriggerAttackFlame(const Vector3i& pos, const Vector3& color, int scale);
void WadeSplash(ItemInfo* item, int wh, int wd);
void Splash(ItemInfo* item);
void TriggerRocketFire(int x, int y, int z);
void TriggerExplosionBubbles(int x, int y, int z, short roomNumber);
void Ricochet(Pose& pos);
void ProcessEffects(ItemInfo* item);
