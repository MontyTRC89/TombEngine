#include "people.h"
#include "..\Global\global.h"
#include "effects.h"
#include "effect2.h"
#include "draw.h"
#include "control.h"
#include "sphere.h"
#include "debris.h"
#include "box.h"

__int32 __cdecl ShotLara(ITEM_INFO* item, AI_INFO* info, BITE_INFO* gun, __int16 extra_rotation, __int32 damage) 
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ITEM_INFO* enemy = creature->enemy;

	__int32 hit = 0;
	__int32 targetable = 0;

	if (info->distance <= SQUARE(8192) && Targetable(item, info))
	{
		__int32 distance = SIN(info->enemyFacing) * enemy->speed >> W2V_SHIFT * SQUARE(8192) / 300;
		distance = info->distance + SQUARE(distance);
		if (distance <= SQUARE(8192))
		{
			__int32 random = (SQUARE(8192) - info->distance) / (SQUARE(8192) / 0x5000) + 8192;
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
				LaraItem->hitPoints -= damage;
				LaraItem->hitStatus = true; 
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
				enemy->hitStatus = true;
				enemy->hitPoints += damage / -10;

				__int32 random = GetRandomControl() & 0xF;
				if (random > 14)
					random = 0;

				PHD_VECTOR pos;
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;

				GetJointAbsPosition(enemy, &pos, random);
				DoBloodSplat(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 4, enemy->pos.yRot, enemy->roomNumber);
			}
		}
	}

	// TODO: smash objects

	return targetable;
}

__int16 __cdecl GunMiss(__int32 x, __int32 y, __int32 z, __int16 speed, __int16 yrot, __int16 roomNumber)
{
	GAME_VECTOR pos;

	pos.x = LaraItem->pos.xPos + ((GetRandomControl() - 0x4000) << 9) / 0x7FFF;
	pos.y = LaraItem->floor;
	pos.z = LaraItem->pos.zPos + ((GetRandomControl() - 0x4000) << 9) / 0x7FFF;
	pos.roomNumber = LaraItem->roomNumber;

	//Richochet(&pos);

	return GunShot(x, y, z, speed, yrot, roomNumber);
}

__int16 __cdecl GunHit(__int32 x, __int32 y, __int32 z, __int16 speed, __int16 yrot, __int16 roomNumber)
{
	PHD_VECTOR pos;

	pos.x = 0;
	pos.y = 0;
	pos.z = 0;

	GetJointAbsPosition(LaraItem, &pos, (25 * GetRandomControl()) >> 15);

	DoBloodSplat(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 3, LaraItem->pos.yRot, LaraItem->roomNumber);
	SoundEffect(SFX_LARA_INJURY_RND, &LaraItem->pos, 0);

	return GunShot(x, y, z, speed, yrot, roomNumber);
}

__int16 __cdecl GunShot(__int32 x, __int32 y, __int32 z, __int16 speed, __int16 yrot, __int16 roomNumber)
{
	return -1;
}

__int32 __cdecl Targetable(ITEM_INFO* item, AI_INFO* info) 
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ITEM_INFO* enemy = creature->enemy;

	if (enemy != NULL && enemy->hitPoints > 0 && enemy->data != NULL && info->ahead && info->distance < SQUARE(2048))
	{
		GAME_VECTOR start, target;

		__int16* bounds = GetBestFrame(item);

		start.x = item->pos.xPos;
		if (item->objectNumber == ID_SNIPER)
			start.y = item->pos.yPos - 768;
		else
			start.y = item->pos.yPos + ((bounds[3] + 3 * bounds[2]) >> 2);
		start.z = item->pos.zPos;
		start.roomNumber = item->roomNumber;

		bounds = GetBestFrame(enemy);

		target.x = enemy->pos.xPos;
		target.y = enemy->pos.yPos + ((bounds[3] + 3 * bounds[2]) >> 2);
		target.z = enemy->pos.zPos;

		return LOS(&start, &target);
	}

	return 0;
}

__int32 __cdecl TargetVisible(ITEM_INFO* item, AI_INFO* info) 
{
	CREATURE_INFO*  creature = (CREATURE_INFO*)item->data;
	ITEM_INFO* enemy = creature->enemy;

	if (enemy != NULL)
	{
		__int16 angle = info->angle - creature->jointRotation[2];
		if (enemy->hitPoints != 0 && angle > -ANGLE(45) && angle < ANGLE(45) && info->distance < SQUARE(8192))
		{
			GAME_VECTOR start;
			start.x = item->pos.xPos;
			start.y = item->pos.yPos - 768;
			start.z = item->pos.zPos;
			start.roomNumber = item->roomNumber;

			GAME_VECTOR target;
			target.x = enemy->pos.xPos;
			__int16* bounds = GetBestFrame(enemy);
			target.y = enemy->pos.yPos + ((((bounds[2] << 1) + bounds[2]) + bounds[3]) >> 2);
			target.z = enemy->pos.zPos;

			return LOS(&start, &target);
		}
	}

	return 0;
}

void Inject_People()
{
	INJECT(0x00467610, ShotLara);
	INJECT(0x00467530, GunMiss);
	INJECT(0x004673D1, GunHit);
	INJECT(0x00467420, GunShot);
	INJECT(0x004672F0, Targetable);
	INJECT(0x004671E0, TargetVisible);
}