#include "framework.h"
#include "people.h"
#include "effect.h"
#include "effect2.h"
#include "draw.h"
#include "control.h"
#include "sphere.h"
#include "debris.h"
#include "lara.h"
#include "sound.h"

int ShotLara(ITEM_INFO* item, AI_INFO* info, BITE_INFO* gun, short extra_rotation, int damage) 
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ITEM_INFO* enemy = creature->enemy;

	int hit = 0;
	int targetable = 0;

	if (info->distance <= SQUARE(8192) && Targetable(item, info))
	{
		int distance = phd_sin(info->enemyFacing) * enemy->speed >> W2V_SHIFT * SQUARE(8192) / 300;
		distance = info->distance + SQUARE(distance);
		if (distance <= SQUARE(8192))
		{
			int random = (SQUARE(8192) - info->distance) / (SQUARE(8192) / 0x5000) + 8192;
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

				int random = GetRandomControl() & 0xF;
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

short GunMiss(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	GAME_VECTOR pos;

	pos.x = LaraItem->pos.xPos + ((GetRandomControl() - 0x4000) << 9) / 0x7FFF;
	pos.y = LaraItem->floor;
	pos.z = LaraItem->pos.zPos + ((GetRandomControl() - 0x4000) << 9) / 0x7FFF;
	pos.roomNumber = LaraItem->roomNumber;

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

	DoBloodSplat(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 3, LaraItem->pos.yRot, LaraItem->roomNumber);
	SoundEffect(SFX_LARA_INJURY_RND, &LaraItem->pos, 0);

	return GunShot(x, y, z, speed, yrot, roomNumber);
}

short GunShot(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	return -1;
}

int Targetable(ITEM_INFO* item, AI_INFO* info) 
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ITEM_INFO* enemy = creature->enemy;

	if (enemy != NULL && enemy->hitPoints > 0 && enemy->data != NULL && info->ahead && info->distance < SQUARE(8192))
	{
		GAME_VECTOR start, target;

		short* bounds = GetBestFrame(item);

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

int TargetVisible(ITEM_INFO* item, AI_INFO* info) 
{
	CREATURE_INFO*  creature = (CREATURE_INFO*)item->data;
	ITEM_INFO* enemy = creature->enemy;

	if (enemy != NULL)
	{
		short angle = info->angle - creature->jointRotation[2];
		if (enemy->hitPoints != 0 && angle > -ANGLE(45) && angle < ANGLE(45) && info->distance < SQUARE(8192))
		{
			GAME_VECTOR start;
			start.x = item->pos.xPos;
			start.y = item->pos.yPos - 768;
			start.z = item->pos.zPos;
			start.roomNumber = item->roomNumber;

			GAME_VECTOR target;
			target.x = enemy->pos.xPos;
			short* bounds = GetBestFrame(enemy);
			target.y = enemy->pos.yPos + ((((bounds[2] << 1) + bounds[2]) + bounds[3]) >> 2);
			target.z = enemy->pos.zPos;

			return LOS(&start, &target);
		}
	}

	return 0;
}