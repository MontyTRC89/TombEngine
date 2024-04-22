#pragma once
#include "Game/items.h"
#include "Math/Math.h"

using namespace TEN::Math;

constexpr auto MAX_WATERFALL_PARTICLES = 16384;

enum class LaraWeaponType;
enum GAME_OBJECT_ID : short;
struct CollisionInfo;
struct ItemInfo;

struct WaterfallParticle
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
	int Life;
	BlendMode blendMode;
	unsigned char extras;
	signed char dynamic;
	int fxObj;
	int roomNumber;
	unsigned char nodeNumber; // ParticleNodeOffsetIDs enum.
	GameVector targetPos;
};

extern std::vector<WaterfallParticle> WaterfallParticles;

WaterfallParticle GetFreeWaterfallParticle();

	void InitializeWaterfall(short itemNumber);
	void TriggerWaterfallEmitterMist(const Vector3& pos, short room);
	void WaterfallControl(short itemNumber);

