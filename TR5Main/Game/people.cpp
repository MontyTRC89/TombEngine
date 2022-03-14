#include "framework.h"
#include "Game/people.h"

#include "Game/animation.h"
#include "Game/control/los.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/debris.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"

bool ShotLara(ITEM_INFO* item, AI_INFO* info, BITE_INFO* gun, short extra_rotation, int damage) 
{
	auto* creature = GetCreatureInfo(item);
	auto* enemy = creature->Enemy;

	bool hit = false;
	bool targetable = false;

	if (info->distance <= pow(MAX_VISIBILITY_DISTANCE, 2) && Targetable(item, info))
	{
		int distance = phd_sin(info->enemyFacing) * enemy->Animation.Velocity * pow(MAX_VISIBILITY_DISTANCE, 2) / 300;
		distance = pow(distance, 2) + info->distance;
		if (distance <= pow(MAX_VISIBILITY_DISTANCE, 2))
		{
			int random = (pow(MAX_VISIBILITY_DISTANCE, 2) - info->distance) / (pow(MAX_VISIBILITY_DISTANCE, 2) / 0x5000) + 8192;
			hit = GetRandomControl() < random;
		}
		else
			hit = false;
		
		targetable = true;
	}
	else
	{
		hit = false;
		targetable = false;
	}

	if (damage)
	{
		if (enemy == LaraItem)
		{
			if (hit)
			{
				CreatureEffect(item, gun, &GunHit);
				LaraItem->HitPoints -= damage;
				LaraItem->HitStatus = true; 
			}
			else if (targetable)
				CreatureEffect(item, gun, &GunMiss);
		}
		else
		{
			CreatureEffect(item, gun, &GunShot);
			if (hit)
			{
				enemy->HitStatus = true;
				enemy->HitPoints += damage / -10;

				int random = GetRandomControl() & 0xF;
				if (random > 14)
					random = 0;

				PHD_VECTOR pos = { 0, 0, 0 };
				GetJointAbsPosition(enemy, &pos, random);
				DoBloodSplat(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 4, enemy->Position.yRot, enemy->RoomNumber);
			}
		}
	}

	// TODO: smash objects

	return targetable;
}

short GunMiss(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	GAME_VECTOR pos;

	pos.x = LaraItem->Position.xPos + ((GetRandomControl() - 0x4000) << 9) / 0x7FFF;
	pos.y = LaraItem->Floor;
	pos.z = LaraItem->Position.zPos + ((GetRandomControl() - 0x4000) << 9) / 0x7FFF;
	pos.roomNumber = LaraItem->RoomNumber;

	Richochet((PHD_3DPOS*)&pos);

	return GunShot(x, y, z, speed, yrot, roomNumber);
}

short GunHit(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	PHD_VECTOR pos = { 0, 0, 0 };
	GetLaraJointPosition(&pos, (25 * GetRandomControl()) >> 15);

	DoBloodSplat(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 3, LaraItem->Position.yRot, LaraItem->RoomNumber);
	SoundEffect(SFX_TR4_LARA_INJURY, &LaraItem->Position, 0);

	return GunShot(x, y, z, speed, yrot, roomNumber);
}

short GunShot(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	return -1;
}

bool Targetable(ITEM_INFO* item, AI_INFO* info) 
{
	auto* creature = GetCreatureInfo(item);
	auto* enemy = creature->Enemy;

	if (enemy == NULL || enemy->HitPoints <= 0 || !info->ahead || info->distance >= pow(MAX_VISIBILITY_DISTANCE, 2))
		return false;

	if (!enemy->Data.is<CreatureInfo>() && !enemy->Data.is<LaraInfo*>())
		return false;

	BOUNDING_BOX* bounds = (BOUNDING_BOX*)GetBestFrame(item);

	GAME_VECTOR start;
	start.x = item->Position.xPos;
	start.y = (item->ObjectNumber == ID_SNIPER) ? (item->Position.yPos - CLICK(3)) : (item->Position.yPos + ((bounds->Y2 + 3 * bounds->Y1) / 4));
	start.z = item->Position.zPos;
	start.roomNumber = item->RoomNumber;

	bounds = (BOUNDING_BOX*)GetBestFrame(enemy);

	GAME_VECTOR target;
	target.x = enemy->Position.xPos;
	target.y = enemy->Position.yPos + ((bounds->Y2 + 3 * bounds->Y1) / 4);
	target.z = enemy->Position.zPos;

	return LOS(&start, &target);
}

bool TargetVisible(ITEM_INFO* item, AI_INFO* info) 
{
	auto* creature = GetCreatureInfo(item);
	auto* enemy = creature->Enemy;

	if (enemy != NULL)
	{
		short angle = info->angle - creature->JointRotation[2];
		if (enemy->HitPoints != 0 && angle > -ANGLE(45.0f) && angle < ANGLE(45.0f) && info->distance < pow(MAX_VISIBILITY_DISTANCE, 2))
		{
			GAME_VECTOR start;
			start.x = item->Position.xPos;
			start.y = item->Position.yPos - CLICK(3);
			start.z = item->Position.zPos;
			start.roomNumber = item->RoomNumber;

			GAME_VECTOR target;
			target.x = enemy->Position.xPos;
			BOUNDING_BOX* bounds = (BOUNDING_BOX*)GetBestFrame(enemy);
			target.y = enemy->Position.yPos + ((((bounds->Y1 * 2) + bounds->Y1) + bounds->Y2) / 4);
			target.z = enemy->Position.zPos;

			return LOS(&start, &target);
		}
	}

	return false;
}
