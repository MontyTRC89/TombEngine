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
#include "Sound/sound.h"

int ShotLara(ITEM_INFO* item, AI_INFO* info, BITE_INFO* gun, short extra_rotation, int damage) 
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	ITEM_INFO* enemy = creature->enemy;

	int hit = 0;
	int targetable = 0;

	if (info->distance <= SQUARE(MAX_VISIBILITY_DISTANCE) && Targetable(item, info))
	{
		int distance = phd_sin(info->enemyFacing) * enemy->Velocity * SQUARE(MAX_VISIBILITY_DISTANCE) / 300;
		distance = info->distance + SQUARE(distance);
		if (distance <= SQUARE(MAX_VISIBILITY_DISTANCE))
		{
			int random = (SQUARE(MAX_VISIBILITY_DISTANCE) - info->distance) / (SQUARE(MAX_VISIBILITY_DISTANCE) / 0x5000) + 8192;
			hit = (GetRandomControl() < random);
		}
		else
		{
			hit = 0;
		}
		targetable = 1;
	}
	else
	{
		hit = 0;
		targetable = 0;
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
			{
				CreatureEffect(item, gun, &GunMiss);
			}
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

				PHD_VECTOR pos;
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;

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
	PHD_VECTOR pos;

	pos.x = 0;
	pos.y = 0;
	pos.z = 0;

	GetLaraJointPosition(&pos, (25 * GetRandomControl()) >> 15);

	DoBloodSplat(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 3, LaraItem->Position.yRot, LaraItem->RoomNumber);
	SoundEffect(SFX_TR4_LARA_INJURY, &LaraItem->Position, 0);

	return GunShot(x, y, z, speed, yrot, roomNumber);
}

short GunShot(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	return -1;
}

int Targetable(ITEM_INFO* item, AI_INFO* info) 
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	ITEM_INFO* enemy = creature->enemy;

	if (enemy == NULL || enemy->HitPoints <= 0 || !info->ahead || info->distance >= SQUARE(MAX_VISIBILITY_DISTANCE))
		return 0;

	if (!enemy->Data.is<CREATURE_INFO>() && !enemy->Data.is<LaraInfo*>())
		return 0;

	GAME_VECTOR start, target;

	BOUNDING_BOX* bounds = (BOUNDING_BOX*)GetBestFrame(item);

	start.x = item->Position.xPos;
	if (item->ObjectNumber == ID_SNIPER)
		start.y = item->Position.yPos - 768;
	else
		start.y = item->Position.yPos + ((bounds->Y2 + 3 * bounds->Y1) / 4);
	start.z = item->Position.zPos;
	start.roomNumber = item->RoomNumber;

	bounds = (BOUNDING_BOX*)GetBestFrame(enemy);

	target.x = enemy->Position.xPos;
	target.y = enemy->Position.yPos + ((bounds->Y2 + 3 * bounds->Y1) / 4);
	target.z = enemy->Position.zPos;

	return LOS(&start, &target);
}

int TargetVisible(ITEM_INFO* item, AI_INFO* info) 
{
	CREATURE_INFO*  creature = (CREATURE_INFO*)item->Data;
	ITEM_INFO* enemy = creature->enemy;

	if (enemy != NULL)
	{
		short angle = info->angle - creature->jointRotation[2];
		if (enemy->HitPoints != 0 && angle > -ANGLE(45) && angle < ANGLE(45) && info->distance < SQUARE(MAX_VISIBILITY_DISTANCE))
		{
			GAME_VECTOR start;
			start.x = item->Position.xPos;
			start.y = item->Position.yPos - 768;
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

	return 0;
}