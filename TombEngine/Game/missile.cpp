#include "framework.h"
#include "Game/missile.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define SHARD_DAMAGE 30
#define ROCKET_DAMAGE 100
#define DIVER_HARPOON_DAMAGE 50

#define SHARD_VELOCITY     250
#define ROCKET_VELOCITY    220
#define NATLA_GUN_VELOCITY 400

void ShootAtLara(FX_INFO *fx)
{
	int x = LaraItem->Pose.Position.x - fx->pos.Position.x;
	int y = LaraItem->Pose.Position.y - fx->pos.Position.y;
	int z = LaraItem->Pose.Position.z - fx->pos.Position.z;

	auto* bounds = GetBoundsAccurate(LaraItem);
	y += bounds->Y2 + (bounds->Y1 - bounds->Y2) * 0.75f;

	int distance = sqrt(pow(x, 2) + pow(z, 2));
	fx->pos.Orientation.x = -phd_atan(distance, y);
	fx->pos.Orientation.y = phd_atan(z, x);

	// Random scatter (only a little bit else it's too hard to avoid).
	fx->pos.Orientation.x += (GetRandomControl() - 0x4000) / 0x40;
	fx->pos.Orientation.y += (GetRandomControl() - 0x4000) / 0x40;
}

void ControlMissile(short fxNumber)
{
	auto* fx = &EffectList[fxNumber];

	if (fx->objectNumber == ID_SCUBA_HARPOON && !TestEnvironment(ENV_FLAG_WATER, fx->roomNumber) && fx->pos.Orientation.x > -0x3000)
	{
		fx->pos.Orientation.x -= ANGLE(1.0f);
	}

	fx->pos.Position.y += fx->speed * phd_sin(-fx->pos.Orientation.x);
	int velocity = fx->speed * phd_cos(fx->pos.Orientation.x);
	fx->pos.Position.z += velocity * phd_cos(fx->pos.Orientation.y);
	fx->pos.Position.x += velocity * phd_sin(fx->pos.Orientation.y);

	auto probe = GetCollision(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, fx->roomNumber);

	// Check for hitting something.
	if (fx->pos.Position.y >= probe.Position.Floor ||
		fx->pos.Position.y <= probe.Position.Ceiling)
	{
		if (/*fx->objectNumber == KNIFE ||*/ fx->objectNumber == ID_SCUBA_HARPOON)
		{
			// Change shard into ricochet.
			//			fx->speed = 0;
			//			fx->frameNumber = -GetRandomControl()/11000;
			//			fx->counter = 6;
			//			fx->objectNumber = RICOCHET1;
			SoundEffect((fx->objectNumber == ID_SCUBA_HARPOON) ? SFX_TR4_WEAPON_RICOCHET : SFX_TR2_CIRCLE_BLADE_HIT, &fx->pos);
		}
		/*else if (fx->objectNumber == DRAGON_FIRE)
		{
			AddDynamicLight(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, 14, 11);
			KillEffect(fx_number);
		}*/
		return;
	}

	if (probe.RoomNumber != fx->roomNumber)
		EffectNewRoom(fxNumber, probe.RoomNumber);

	// Check for hitting Lara.
	/*if (fx->objectNumber == DRAGON_FIRE)
	{
		if (ItemNearLara(&fx->pos, 350))
		{
			DoDamage(LaraItem, 3);
			LaraBurn(LaraItem);
			return;
		}
	}*/
	else if (ItemNearLara(&fx->pos, 200))
	{
		/*if (fx->objectNumber == KNIFE)
		{
			DoDamage(LaraItem, KNIFE_DAMAGE);
			KillEffect(fx_number);
		}
		else*/ if (fx->objectNumber == ID_SCUBA_HARPOON)
		{
			DoDamage(LaraItem, DIVER_HARPOON_DAMAGE);
			KillEffect(fxNumber);
		}

		LaraItem->HitStatus = 1;

		fx->pos.Orientation.y = LaraItem->Pose.Orientation.y;
		fx->speed = LaraItem->Animation.Velocity;
		fx->frameNumber = fx->counter = 0;
	}

	// Create bubbles in wake of harpoon bolt.
	//if (fx->objectNumber == ID_SCUBA_HARPOON && g_Level.Rooms[fx->roomNumber].flags & 1)
	//	CreateBubble(&fx->pos, fx->roomNumber, 1, 0);
	/*else if (fx->objectNumber == DRAGON_FIRE && !fx->counter--)
	{
		AddDynamicLight(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, 14, 11);
		SoundEffect(305, &fx->pos);
		KillEffect(fx_number);
	}
	else if (fx->objectNumber == KNIFE)
		fx->pos.Orientation.z += 30 * ONE_DEGREE;*/
}

void ControlNatlaGun(short fxNumber)
{
	auto* fx = &EffectList[fxNumber];
	auto* object = &Objects[fx->objectNumber];

	fx->frameNumber--;
	if (fx->frameNumber <= Objects[fx->objectNumber].nmeshes)
		KillEffect(fxNumber);

	/* If first frame, then start another explosion at next position */
	if (fx->frameNumber == -1)
	{
		int z = fx->pos.Position.z + fx->speed * phd_cos(fx->pos.Orientation.y);
		int x = fx->pos.Position.x + fx->speed * phd_sin(fx->pos.Orientation.y);
		int y = fx->pos.Position.y;

		auto probe = GetCollision(x, y, z, fx->roomNumber);

		// Don't create one if hit a wall.
		if (y >= probe.Position.Floor ||
			y <= probe.Position.Ceiling)
		{
			return;
		}

		fxNumber = CreateNewEffect(probe.RoomNumber);
		if (fxNumber != NO_ITEM)
		{
			auto* fxNew = &EffectList[fxNumber];

			fxNew->pos.Position.x = x;
			fxNew->pos.Position.y = y;
			fxNew->pos.Position.z = z;
			fxNew->pos.Orientation.y = fx->pos.Orientation.y;
			fxNew->roomNumber = probe.RoomNumber;
			fxNew->speed = fx->speed;
			fxNew->frameNumber = 0;
			fxNew->objectNumber = ID_PROJ_NATLA;
		}
	}
}

short ShardGun(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != NO_ITEM)
	{
		auto* fx = &EffectList[fxNumber];

		fx->pos.Position.x = x;
		fx->pos.Position.y = y;
		fx->pos.Position.z = z;
		fx->roomNumber = roomNumber;
		fx->pos.Orientation.x = fx->pos.Orientation.z = 0;
		fx->pos.Orientation.y = yRot;
		fx->speed = SHARD_VELOCITY;
		fx->frameNumber = 0;
		fx->objectNumber = ID_PROJ_SHARD;
		fx->shade = 14 * 256;
		ShootAtLara(fx);
	}

	return fxNumber;
}

short BombGun(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != NO_ITEM)
	{
		auto* fx = &EffectList[fxNumber];

		fx->pos.Position.x = x;
		fx->pos.Position.y = y;
		fx->pos.Position.z = z;
		fx->roomNumber = roomNumber;
		fx->pos.Orientation.x = fx->pos.Orientation.z = 0;
		fx->pos.Orientation.y = yRot;
		fx->speed = ROCKET_VELOCITY;
		fx->frameNumber = 0;
		fx->objectNumber = ID_PROJ_BOMB;
		fx->shade = 16 * 256;
		ShootAtLara(fx);
	}

	return fxNumber;
}

short NatlaGun(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != NO_ITEM)
	{
		auto* fx = &EffectList[fxNumber];

		fx->pos.Position.x = x;
		fx->pos.Position.y = y;
		fx->pos.Position.z = z;
		fx->roomNumber = roomNumber;
		fx->pos.Orientation.x = fx->pos.Orientation.z = 0;
		fx->pos.Orientation.y = yRot;
		fx->speed = NATLA_GUN_VELOCITY;
		fx->frameNumber = 0;
		fx->objectNumber = ID_PROJ_NATLA;
		fx->shade = 16 * 256;
		ShootAtLara(fx);
	}

	return fxNumber;
}
