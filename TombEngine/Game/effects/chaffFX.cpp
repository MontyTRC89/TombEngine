#include "framework.h"
#include "Game/effects/chaffFX.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/Bubble.h"
#include "Game/effects/smoke.h"
#include "Game/effects/spark.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Renderer/Renderer11Enums.h"
#include "Sound/sound.h"

using namespace TEN::Effects::Bubble;
using namespace TEN::Math;

constexpr auto MAX_TRIGGER_RANGE = BLOCK(16);

void TriggerChaffEffects(int flareLife)
{
	auto pos = GetJointPosition(LaraItem, LM_LHAND, Vector3i(8, 36, 32));
	auto vect = GetJointPosition(LaraItem, LM_LHAND, Vector3i(8, 36, 1024 + Random::GenerateInt(0, 256)));
	auto vel = vect - pos;
	TriggerChaffEffects(*LaraItem, pos, vel, LaraItem->Animation.Velocity.z, TestEnvironment(ENV_FLAG_WATER, LaraItem->RoomNumber), flareLife);
}

void TriggerChaffEffects(ItemInfo& item, int age)
{
	auto world = Matrix::CreateTranslation(-6, 6, 32) * item.Pose.Orientation.ToRotationMatrix();
	auto pos = item.Pose.Position + Vector3i(world.Translation());

	world = Matrix::CreateTranslation(-6, 6, 32) *
		Matrix::CreateTranslation((GetRandomDraw() & 127) - 64, (GetRandomDraw() & 127) - 64, (GetRandomDraw() & 511) + 512) *
		item.Pose.Orientation.ToRotationMatrix();

	auto vel = Vector3i(world.Translation());
	TriggerChaffEffects(item, pos, vel, item.Animation.Velocity.z, TestEnvironment(ENV_FLAG_WATER, &item), age);
}

void TriggerChaffEffects(ItemInfo& item, const Vector3i& pos, const Vector3i& vel, int speed, bool isUnderwater, int age)
{
	auto pose = item.Pose;
	if (item.IsLara())
	{
		auto handPos = GetJointPosition(&item, LM_RHAND);
		pose.Position = handPos;
		pose.Position.y -= 64;
	}

	auto cond = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, pose.Position, item.RoomNumber);
	SoundEffect(cond ? SFX_TR4_FLARE_BURN_UNDERWATER : SFX_TR4_FLARE_BURN_DRY, &pose, SoundEnvironment::Always, 1.0f, 0.5f);

	if (!age)
		return;

	int numSparks = Random::GenerateInt(1, 3);

	for (int i = 0; i < numSparks; i++)
	{
		long dx, dz;

		dx = item.Pose.Position.x - pos.x;
		dz = item.Pose.Position.z - pos.z;

		if (dx < -MAX_TRIGGER_RANGE || dx > MAX_TRIGGER_RANGE || dz < -MAX_TRIGGER_RANGE || dz > MAX_TRIGGER_RANGE)
			return;

		ColorData color;
		color.r = 255;
		color.g = (GetRandomDraw() & 127) + 64;
		color.b = 192 - color.g;

		TriggerChaffSparkles(pos, vel, color, age, item);

		if (isUnderwater)
		{
			TriggerChaffBubbles(pos, item.RoomNumber);
		}
		else
		{
			auto direction = vel.ToVector3();
			direction.Normalize();
			TEN::Effects::Smoke::TriggerFlareSmoke(pos.ToVector3() + direction * 20, direction, age, item.RoomNumber);
		}
	}
}

void TriggerChaffSparkles(const Vector3i& pos, const Vector3i& vel, const ColorData& color, int age, const ItemInfo& item)
{
	TEN::Effects::Spark::TriggerFlareSparkParticles(pos, vel, color, item.RoomNumber);
}

void TriggerChaffSmoke(const Vector3i& pos, const Vector3i& vel, int speed, bool isMoving, bool wind)
{
	SMOKE_SPARKS* smoke;

	int rnd = 0;
	BYTE trans, size;
	
	smoke = &SmokeSparks[GetFreeSmokeSpark()];

	smoke->on = true;

	smoke->sShade = 0;
	if (isMoving)
	{
		trans = (speed << 7) >> 5;
		smoke->dShade = trans;
	}
	else
		smoke->dShade = 64 + (GetRandomDraw() & 7);

	smoke->colFadeSpeed = 4 + (GetRandomDraw() & 3);
	smoke->fadeToBlack = 4;

	rnd = (GetRandomControl() & 3) - (speed >> 12) + 20;
	if (rnd < 9)
	{
		smoke->life = 9;
		smoke->sLife = 9;
	}
	else
	{
		smoke->life = rnd;
		smoke->sLife = rnd;
	}

	smoke->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
	
	smoke->x = pos.x + (GetRandomControl() & 7) - 3;
	smoke->y = pos.y + (GetRandomControl() & 7) - 3;
	smoke->z = pos.z + (GetRandomControl() & 7) - 3;
	smoke->xVel = vel.x + ((GetRandomDraw() & 63) - 32);
	smoke->yVel = vel.y;
	smoke->zVel = vel.z + ((GetRandomDraw() & 63) - 32);
	smoke->friction = 4;

	if (GetRandomControl() & 1)
	{
		smoke->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE;
		smoke->rotAng = (GetRandomControl() & 0xFFF);
		if (GetRandomControl() & 1)
			smoke->rotAdd = (GetRandomControl() & 7) - 24;
		else
			smoke->rotAdd = (GetRandomControl() & 7) + 24;
	}
	else
		smoke->flags = SP_EXPDEF | SP_DEF | SP_SCALE;

	if (wind)
		smoke->flags |= SP_WIND;

	smoke->scalar = 1;
	smoke->gravity = (GetRandomControl() & 3) - 4;
	smoke->maxYvel = 0;
	size = (GetRandomControl() & 7) + (speed >> 7) + 32;
	smoke->sSize = size >> 2;
	smoke->size = smoke->dSize = size;
}

// TODO: Move to Bubble.cpp
void TriggerChaffBubbles(const Vector3i& pos, int roomNumber)
{
	constexpr auto COLOR_END		= Vector4(1.0f, 1.0f, 1.0f, 0.0f);
	constexpr auto OPACTY_MAX		= 0.8f;
	constexpr auto OPACTY_MIN		= 0.3f;
	constexpr auto AMPLITUDE_MAX	= BLOCK(1 / 16.0f);
	constexpr auto SCALE_LARGE_MAX	= BLOCK(0.5f);
	constexpr auto OSC_VELOCITY_MAX = 0.4f;
	constexpr auto OSC_VELOCITY_MIN = 0.1f;

	auto& bubble = GetNewEffect(Bubbles, BUBBLE_COUNT_MAX);

	auto sphere = BoundingSphere(Vector3::Zero, AMPLITUDE_MAX);

	bubble.SpriteIndex = SPR_BUBBLES;
	bubble.Position = pos.ToVector3();
	bubble.PositionBase = bubble.Position;
	bubble.RoomNumber = roomNumber;

	bubble.Color =
	bubble.ColorStart = Vector4(1.0f, 1.0f, 1.0f, Random::GenerateFloat(OPACTY_MIN, OPACTY_MAX));
	bubble.ColorEnd = COLOR_END;
	bubble.Orientation2D = 0;

	bubble.Inertia = Vector3::Zero;
	bubble.Amplitude = Random::GeneratePointInSphere(sphere);
	bubble.WavePeriod = Vector3(Random::GenerateFloat(-PI, PI), Random::GenerateFloat(-PI, PI), Random::GenerateFloat(-PI, PI));
	bubble.WaveVelocity = Vector3(
		1 / Random::GenerateFloat(8, 16),
		1 / Random::GenerateFloat(8, 16),
		1 / Random::GenerateFloat(8, 16));

	bubble.Life = 0.0f;
	bubble.Gravity = Random::GenerateFloat(4.0f, 16.0f);
	bubble.OscillationPeriod = Random::GenerateFloat(0.0f, (bubble.ScaleMax.x + bubble.ScaleMax.y) / 2);
	bubble.OscillationVelocity = Lerp(OSC_VELOCITY_MAX, OSC_VELOCITY_MIN, ((bubble.ScaleMax.x + bubble.ScaleMax.y) / 2) / SCALE_LARGE_MAX);
	bubble.Scale =
	bubble.ScaleMax =
	bubble.ScaleMin = Vector2(Random::GenerateFloat(32.0f, 96.0f));
	bubble.Rotation = 0;
}
