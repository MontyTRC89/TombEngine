#include "framework.h"
#include "Game/missile.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/effects.h"
#include "Game/effects/explosion.h"
#include "Game/effects/Bubble.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Effects::Bubble;
using namespace TEN::Effects::Explosion;
using namespace TEN::Math;

constexpr auto MUTANT_SHARD_DAMAGE	= 30;
constexpr auto MUTANT_BOMB_DAMAGE	= 100;
constexpr auto DIVER_HARPOON_DAMAGE = 50;
constexpr auto KNIFE_DAMAGE			= 50;

void ShootAtLara(ItemInfo& fx)
{
	auto target = Vector3(
		LaraItem->Pose.Position.x,
		LaraItem->Pose.Position.y - (GameBoundingBox(LaraItem).GetHeight() * 0.75f),
		LaraItem->Pose.Position.z);
	fx.Pose.Orientation = Geometry::GetOrientToPoint(fx.Pose.Position.ToVector3(), target);

	// Apply slight random scatter.
	fx.Pose.Orientation += EulerAngles(
		Random::GenerateAngle(ANGLE(-1.4f), ANGLE(1.4f)),
		Random::GenerateAngle(ANGLE(-1.4f), ANGLE(1.4f)),
		0);
}

// TODO: Make ControlMissile() not use LaraItem global. -- TokyoSU 5/8/2022
void ControlMissile(short fxNumber)
{
	constexpr auto HIT_RADIUS = 200;

	// TODO: Add fx.target (ItemInfo* target) to get the actual target (if it was really it).
	auto& fx = g_Level.Items[fxNumber];

	auto isUnderwater = TestEnvironment(ENV_FLAG_WATER, fx.RoomNumber);
	auto soundFXType = isUnderwater ? SoundEnvironment::Water : SoundEnvironment::Land;

	if (fx.ObjectNumber == ID_SCUBA_HARPOON && isUnderwater &&
		fx.Pose.Orientation.x > ANGLE(-67.5f))
	{
		fx.Pose.Orientation.x -= ANGLE(1.0f);
	}

	fx.Pose.Translate(fx.Pose.Orientation, fx.Animation.Velocity.z);

	auto pointColl = GetCollision(fx.Pose.Position.x, fx.Pose.Position.y, fx.Pose.Position.z, fx.RoomNumber);
	auto hasHitPlayer = ItemNearLara(fx.Pose.Position, HIT_RADIUS);

	// Check whether something was hit.
	if (fx.Pose.Position.y >= pointColl.Position.Floor ||
		fx.Pose.Position.y <= pointColl.Position.Ceiling ||
		hasHitPlayer)
	{
		if (fx.ObjectNumber == ID_KNIFETHROWER_KNIFE ||
			fx.ObjectNumber == ID_SCUBA_HARPOON ||
			fx.ObjectNumber == ID_PROJ_SHARD)
		{
			SoundEffect((fx.ObjectNumber == ID_SCUBA_HARPOON) ? SFX_TR4_WEAPON_RICOCHET : SFX_TR2_CIRCLE_BLADE_HIT, &fx.Pose, soundFXType);
		}
		else if (fx.ObjectNumber == ID_PROJ_BOMB)
		{
			SoundEffect(SFX_TR1_ATLANTEAN_EXPLODE, &fx.Pose, soundFXType);
			TriggerExplosionSparks(fx.Pose.Position.x, fx.Pose.Position.y, fx.Pose.Position.z, 3, -2, 0, fx.RoomNumber);
			TriggerExplosionSparks(fx.Pose.Position.x, fx.Pose.Position.y, fx.Pose.Position.z, 3, -1, 0, fx.RoomNumber);
			TriggerShockwave(&fx.Pose, 48, 304, (GetRandomControl() & 0x1F) + 112, 128, 32, 32, 32, EulerAngles(ANGLE(12.0f), 0, 0), 0, true, false, (int)ShockwaveStyle::Normal);
		}

		if (hasHitPlayer)
		{
			if (fx.ObjectNumber == ID_KNIFETHROWER_KNIFE)
			{
				DoDamage(LaraItem, KNIFE_DAMAGE);
			}
			else if (fx.ObjectNumber == ID_SCUBA_HARPOON)
			{
				DoDamage(LaraItem, DIVER_HARPOON_DAMAGE);
			}
			else if (fx.ObjectNumber == ID_PROJ_BOMB)
			{
				DoDamage(LaraItem, MUTANT_BOMB_DAMAGE);
			}
			else if (fx.ObjectNumber == ID_PROJ_SHARD)
			{
				TriggerBlood(fx.Pose.Position.x, fx.Pose.Position.y, fx.Pose.Position.z, 0, 10);
				SoundEffect(SFX_TR4_BLOOD_LOOP, &fx.Pose, soundFXType);
				DoDamage(LaraItem, MUTANT_SHARD_DAMAGE);
			}

			LaraItem->HitStatus = true;
			fx.Pose.Orientation.y = LaraItem->Pose.Orientation.y;
			fx.Animation.Velocity.z = LaraItem->Animation.Velocity.z;
			fx.Animation.FrameNumber = 0;
			GetFXInfo(fx).Counter = 0;
		}

		KillEffect(fxNumber);
	}

	if (pointColl.RoomNumber != fx.RoomNumber)
		EffectNewRoom(fxNumber, pointColl.RoomNumber);

	if (fx.ObjectNumber == ID_KNIFETHROWER_KNIFE)
		fx.Pose.Orientation.z += ANGLE(30.0f); // Update knife rotation over time.

	switch (fx.ObjectNumber)
	{
	case ID_SCUBA_HARPOON:
		if (TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx.RoomNumber))
			SpawnBubble(fx.Pose.Position.ToVector3(), fx.RoomNumber);

		break;

	case ID_PROJ_BOMB:
		TriggerDynamicLight(fx.Pose.Position.x, fx.Pose.Position.y, fx.Pose.Position.z, 14, 180, 100, 0);
		break;
	}
}

void ControlNatlaGun(short fxNumber)
{
	auto& fx = g_Level.Items[fxNumber];

	fx.Animation.FrameNumber--;
	if (fx.Animation.FrameNumber <= Objects[fx.ObjectNumber].nmeshes)
		KillEffect(fxNumber);

	// If first frame, start another explosion at next position.
	if (fx.Animation.FrameNumber == -1)
	{
		auto pointColl = GetCollision(fx.Pose.Position, fx.RoomNumber, fx.Pose.Orientation.y, fx.Animation.Velocity.z);

		// Don't create one if hit a wall.
		if (pointColl.Coordinates.y >= pointColl.Position.Floor ||
			pointColl.Coordinates.y <= pointColl.Position.Ceiling)
		{
			return;
		}

		fxNumber = CreateNewEffect(pointColl.RoomNumber, ID_PROJ_BOMB, fx.Pose.Position);
		if (fxNumber == NO_ITEM)
			return;

		auto& fxNew = g_Level.Items[fxNumber];

		fxNew.Pose.Position = pointColl.Coordinates;
		fxNew.Pose.Orientation.y = fx.Pose.Orientation.y;
		fxNew.RoomNumber = pointColl.RoomNumber;
		fxNew.Animation.Velocity.z = fx.Animation.Velocity.z;
		fxNew.Animation.FrameNumber = 0;
		fxNew.ObjectNumber = ID_PROJ_BOMB;
	}
}

short ShardGun(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	int fxNumber = CreateNewEffect(roomNumber, ID_PROJ_SHARD, Pose(x, y, z));
	if (fxNumber != NO_ITEM)
	{
		auto& fx = g_Level.Items[fxNumber];

		fx.Pose.Position = Vector3i(x, y, z);
		fx.Pose.Orientation = EulerAngles(0, yRot, 0);
		fx.RoomNumber = roomNumber;
		fx.Animation.Velocity.z = velocity;
		fx.Animation.FrameNumber = 0;
		fx.ObjectNumber = ID_PROJ_SHARD;
		fx.Model.Color = Vector4::One;

		ShootAtLara(fx);
	}

	return fxNumber;
}

short BombGun(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	int fxNumber = CreateNewEffect(roomNumber, ID_PROJ_BOMB, Pose(x, y, z));
	if (fxNumber != NO_ITEM)
	{
		auto& fx = g_Level.Items[fxNumber];

		fx.Pose.Position = Vector3i(x, y, z);
		fx.Pose.Orientation = EulerAngles(0, yRot, 0);
		fx.RoomNumber = roomNumber;
		fx.Animation.Velocity.z = velocity;
		fx.Animation.FrameNumber = 0;
		fx.ObjectNumber = ID_PROJ_BOMB;
		fx.Model.Color = Vector4::One;

		ShootAtLara(fx);
	}

	return fxNumber;
}
