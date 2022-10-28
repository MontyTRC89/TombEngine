#include "framework.h"
#include "Game/missile.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/effects.h"
#include "Game/effects/explosion.h"
#include "Game/effects/bubble.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Explosion;
using namespace TEN::Math;

constexpr auto MUTANT_SHARD_DAMAGE	= 30;
constexpr auto MUTANT_BOMB_DAMAGE	= 100;
constexpr auto DIVER_HARPOON_DAMAGE = 50;
constexpr auto KNIFE_DAMAGE			= 50;

void ShootAtLara(FX_INFO& fx)
{
	auto target = Vector3(
		LaraItem->Pose.Position.x,
		LaraItem->Pose.Position.y + (GameBoundingBox(LaraItem).GetHeight() * 0.75f),
		LaraItem->Pose.Position.z
	);
	fx.pos.Orientation = Geometry::GetOrientToPoint(fx.pos.Position.ToVector3(), target);

	// Apply slight random scatter.
	fx.pos.Orientation += EulerAngles(
		Random::GenerateAngle(ANGLE(-1.4f), ANGLE(1.4f)),
		Random::GenerateAngle(ANGLE(-1.4f), ANGLE(1.4f)),
		0
	);
}

// TODO: Make ControlMissile() not use LaraItem global. -- TokyoSU 5/8/2022
void ControlMissile(short fxNumber)
{
	static const int hitRadius = 200;

	// TODO: Add fx.target (ItemInfo* target) to get the actual target (if it was really it).
	auto& fx = EffectList[fxNumber];

	auto isUnderwater = TestEnvironment(ENV_FLAG_WATER, fx.roomNumber);
	auto soundFXType = isUnderwater ? SoundEnvironment::Water : SoundEnvironment::Land;

	if (fx.objectNumber == ID_SCUBA_HARPOON && isUnderwater &&
		fx.pos.Orientation.x > ANGLE(-67.5f))
	{
		fx.pos.Orientation.x -= ANGLE(1.0f);
	}

	fx.pos.Translate(fx.pos.Orientation, fx.speed);

	auto pointColl = GetCollision(fx.pos.Position.x, fx.pos.Position.y, fx.pos.Position.z, fx.roomNumber);
	auto hasHitPlayer = ItemNearLara(fx.pos.Position, hitRadius);

	// Check whether something was hit.
	if (fx.pos.Position.y >= pointColl.Position.Floor ||
		fx.pos.Position.y <= pointColl.Position.Ceiling ||
		hasHitPlayer)
	{
		if (fx.objectNumber == ID_KNIFETHROWER_KNIFE ||
			fx.objectNumber == ID_SCUBA_HARPOON ||
			fx.objectNumber == ID_PROJ_SHARD)
		{
			SoundEffect((fx.objectNumber == ID_SCUBA_HARPOON) ? SFX_TR4_WEAPON_RICOCHET : SFX_TR2_CIRCLE_BLADE_HIT, &fx.pos, soundFXType);
		}
		else if (fx.objectNumber == ID_PROJ_BOMB)
		{
			SoundEffect(SFX_TR1_ATLANTEAN_EXPLODE, &fx.pos, soundFXType);
			TriggerExplosionSparks(fx.pos.Position.x, fx.pos.Position.y, fx.pos.Position.z, 0, false, isUnderwater, fx.roomNumber);
		}

		if (hasHitPlayer)
		{
			if (fx.objectNumber == ID_KNIFETHROWER_KNIFE)
			{
				DoDamage(LaraItem, KNIFE_DAMAGE);
				KillEffect(fxNumber);
			}
			else if (fx.objectNumber == ID_SCUBA_HARPOON)
			{
				DoDamage(LaraItem, DIVER_HARPOON_DAMAGE);
				KillEffect(fxNumber);
			}
			else if (fx.objectNumber == ID_PROJ_BOMB)
			{
				DoDamage(LaraItem, MUTANT_BOMB_DAMAGE);
				KillEffect(fxNumber);
			}
			else if (fx.objectNumber == ID_PROJ_SHARD)
			{
				TriggerBlood(fx.pos.Position.x, fx.pos.Position.y, fx.pos.Position.z, 0, 10);
				SoundEffect(SFX_TR4_BLOOD_LOOP, &fx.pos, soundFXType);
				DoDamage(LaraItem, MUTANT_SHARD_DAMAGE);
				KillEffect(fxNumber);
			}

			LaraItem->HitStatus = true;
			fx.pos.Orientation.y = LaraItem->Pose.Orientation.y;
			fx.speed = LaraItem->Animation.Velocity.z;
			fx.frameNumber = 0;
			fx.counter = 0;
		}
	}

	if (pointColl.RoomNumber != fx.roomNumber)
		EffectNewRoom(fxNumber, pointColl.RoomNumber);

	if (fx.objectNumber == ID_KNIFETHROWER_KNIFE)
		fx.pos.Orientation.z += ANGLE(3.0f); // Update knife rotation over time.

	switch (fx.objectNumber)
	{
	case ID_SCUBA_HARPOON:
		if (TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx.roomNumber))
			CreateBubble(&fx.pos.Position, fx.roomNumber, 1, 0, 0, 0, 0, 0);

		break;

	case ID_PROJ_BOMB:
		TriggerDynamicLight(fx.pos.Position.x, fx.pos.Position.y, fx.pos.Position.z, 14, 180, 100, 0);
		break;
	}
}

void ControlNatlaGun(short fxNumber)
{
	auto& fx = EffectList[fxNumber];

	fx.frameNumber--;
	if (fx.frameNumber <= Objects[fx.objectNumber].nmeshes)
		KillEffect(fxNumber);

	// If first frame, start another explosion at next position.
	if (fx.frameNumber == -1)
	{
		auto pointColl = GetCollision(fx.pos.Position, fx.roomNumber, fx.pos.Orientation.y, fx.speed);

		// Don't create one if hit a wall.
		if (pointColl.Coordinates.y >= pointColl.Position.Floor ||
			pointColl.Coordinates.y <= pointColl.Position.Ceiling)
		{
			return;
		}

		fxNumber = CreateNewEffect(pointColl.RoomNumber);
		if (fxNumber != NO_ITEM)
		{
			auto& fxNew = EffectList[fxNumber];

			fxNew.pos.Position = pointColl.Coordinates;
			fxNew.pos.Orientation.y = fx.pos.Orientation.y;
			fxNew.roomNumber = pointColl.RoomNumber;
			fxNew.speed = fx.speed;
			fxNew.frameNumber = 0;
			fxNew.objectNumber = ID_PROJ_NATLA;
		}
	}
}

short ShardGun(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != NO_ITEM)
	{
		auto& fx = EffectList[fxNumber];

		fx.pos.Position = Vector3i(x, y, z);
		fx.pos.Orientation = EulerAngles(0, yRot, 0);
		fx.roomNumber = roomNumber;
		fx.speed = velocity;
		fx.frameNumber = 0;
		fx.objectNumber = ID_PROJ_SHARD;
		fx.color = Vector4::One;
		ShootAtLara(fx);
	}

	return fxNumber;
}

short BombGun(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != NO_ITEM)
	{
		auto& fx = EffectList[fxNumber];

		fx.pos.Position = Vector3i(x, y, z);
		fx.pos.Orientation = EulerAngles(0, yRot, 0);
		fx.roomNumber = roomNumber;
		fx.speed = velocity;
		fx.frameNumber = 0;
		fx.objectNumber = ID_PROJ_BOMB;
		fx.color = Vector4::One;
		ShootAtLara(fx);
	}

	return fxNumber;
}

short NatlaGun(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != NO_ITEM)
	{
		auto& fx = EffectList[fxNumber];

		fx.pos.Position = Vector3i(x, y, z);
		fx.pos.Orientation = EulerAngles(0, yRot, 0);
		fx.roomNumber = roomNumber;
		fx.speed = velocity;
		fx.frameNumber = 0;
		fx.objectNumber = ID_PROJ_NATLA;
		fx.color = Vector4::One;
		ShootAtLara(fx);
	}

	return fxNumber;
}
