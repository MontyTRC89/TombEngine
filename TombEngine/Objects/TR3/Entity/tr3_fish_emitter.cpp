#include "framework.h"
#include "Objects/TR3/Entity/tr3_fish_emitter.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Objects/TR3/fish.h"
#include "Specific/level.h"

namespace TEN::Entities::TR3
{
	int PirahnaHitWait = false;
	int CarcassItem = NO_ITEM;

	#define PIRAHNA_DAMAGE 4
	#define X 0
	#define Y 1
	#define Z 2
	#define XYZ	3
	#define MAX_FISH 8
	#define OCB_FISH_LETAL 0x1000
	#define OCB_FISH_EAT_CARCASS 0x2000

	FishLeaderInfo LeaderInfo[MAX_FISH];
	FishInfo Fishes[MAX_FISH + (MAX_FISH * 24)];

	unsigned char FishRanges[1][3] =
	{
		{
			8,
			20,
			3
		}
	};

	void SetupShoal(int shoalNumber)
	{
		LeaderInfo[shoalNumber].xRange = (FishRanges[shoalNumber][0] + 2) << 8;
		LeaderInfo[shoalNumber].zRange = (FishRanges[shoalNumber][1] + 2) << 8;
		LeaderInfo[shoalNumber].yRange = (FishRanges[shoalNumber][2]) << 8;
	}

	void SetupFish(int leader, ItemInfo* item)
	{
		int fishXRange = LeaderInfo[leader].xRange;
		int fishYRange = LeaderInfo[leader].yRange;
		int fishZRange = LeaderInfo[leader].zRange;

		Fishes[leader].x = 0;
		Fishes[leader].y = 0;
		Fishes[leader].z = 0;
		Fishes[leader].angle = 0;
		Fishes[leader].speed = (GetRandomControl() & 63) + 8;
		Fishes[leader].swim = GetRandomControl() & 63;

		for (int i = 0; i < 24; i++)
		{
			Fishes[MAX_FISH + (leader * 24) + i].x = (GetRandomControl() % (fishXRange * 2)) - fishXRange;
			Fishes[MAX_FISH + (leader * 24) + i].y = (GetRandomControl() % fishYRange);
			Fishes[MAX_FISH + (leader * 24) + i].destY = (GetRandomControl() % fishYRange);
			Fishes[MAX_FISH + (leader * 24) + i].z = (GetRandomControl() % (fishZRange * 2)) - fishZRange;
			Fishes[MAX_FISH + (leader * 24) + i].angle = GetRandomControl() & 4095;
			Fishes[MAX_FISH + (leader * 24) + i].speed = (GetRandomControl() & 31) + 32;
			Fishes[MAX_FISH + (leader * 24) + i].swim = GetRandomControl() & 63;
		}

		LeaderInfo[leader].on = 1;
		LeaderInfo[leader].angle = 0;
		LeaderInfo[leader].speed = (GetRandomControl() & 127) + 32;
		LeaderInfo[leader].angleTime = 0;
		LeaderInfo[leader].speedTime = 0;
	}

	void ControlFish(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* enemy = item;

		if (!TriggerActive(item))
			return;

		int pirahnaAttack = 0;
		int angle = 0;

		int leader = item->HitPoints;
		if (!LeaderInfo[leader].on)
			SetupFish(leader, item);

		if (item->TriggerFlags & OCB_FISH_LETAL)
		{
			if ((item->TriggerFlags == OCB_FISH_EAT_CARCASS) == 0)
				pirahnaAttack = (LaraItem->RoomNumber == item->RoomNumber);
			else
			{
				if (CarcassItem != -1)
					pirahnaAttack = 2;
				else
					pirahnaAttack = (LaraItem->RoomNumber == item->RoomNumber);
			}
		}
		else
			pirahnaAttack = 0;

		if (PirahnaHitWait)
			PirahnaHitWait--;

		auto* fish = &Fishes[leader];

		if (pirahnaAttack)
		{
			if (pirahnaAttack == 1)
				enemy = LaraItem;
			else
				enemy = &g_Level.Items[CarcassItem];

			LeaderInfo[leader].angle = fish->angle = ((-(mGetAngle(fish->x + item->Pose.Position.x, fish->z + item->Pose.Position.z, enemy->Pose.Position.x, enemy->Pose.Position.z) + 0x4000)) / 16) & 4095;
			LeaderInfo[leader].speed = (GetRandomControl() & 63) + 192;
		}

		int deltaAngle = fish->angle - LeaderInfo[leader].angle;

		if (deltaAngle > 2048)
			deltaAngle -= 4096;
		else if (deltaAngle < -2048)
			deltaAngle += 4096;

		if (deltaAngle > 128)
		{
			fish->angAdd -= 4;
			if (fish->angAdd < -120)
				fish->angAdd = -120;
		}
		else if (deltaAngle < -128)
		{
			fish->angAdd += 4;
			if (fish->angAdd > 120)
				fish->angAdd = 120;
		}
		else
		{
			fish->angAdd -= fish->angAdd / 4;
			if (abs(fish->angAdd) < 4)
				fish->angAdd = 0;
		}

		fish->angle += fish->angAdd;

		if (deltaAngle > 1024)
			fish->angle += fish->angAdd / 4;
		fish->angle &= 4095;

		deltaAngle = fish->speed - LeaderInfo[leader].speed;

		if (deltaAngle < -4)
		{
			deltaAngle = fish->speed + (GetRandomControl() & 3) + 1;
			if (deltaAngle < 0)
				deltaAngle = 0;

			fish->speed = deltaAngle;
		}
		else if (deltaAngle > 4)
		{
			deltaAngle = fish->speed - (GetRandomControl() & 3) - 1;
			if (deltaAngle > 255)
				deltaAngle = 255;

			fish->speed = deltaAngle;
		}

		fish->swim += fish->speed / 16;
		fish->swim &= 63;

		int x = fish->x;
		int z = fish->z;

		x -= fish->speed * phd_sin(fish->angle * 16) / 2;
		z += fish->speed * phd_cos(fish->angle * 16) / 2;

		if (pirahnaAttack == 0)
		{
			int fishXRange = LeaderInfo[leader].xRange;
			int fishZRange = LeaderInfo[leader].zRange;

			if (z < -fishZRange)
			{
				z = -fishZRange;

				if (fish->angle < 2048)
					LeaderInfo[leader].angle = fish->angle - ((GetRandomControl() & 127) + 128);
				else
					LeaderInfo[leader].angle = fish->angle + ((GetRandomControl() & 127) + 128);

				LeaderInfo[leader].angleTime = (GetRandomControl() & 15) + 8;
				LeaderInfo[leader].speedTime = 0;
			}
			else if (z > fishZRange)
			{
				z = fishZRange;

				if (fish->angle > 3072)
					LeaderInfo[leader].angle = fish->angle - ((GetRandomControl() & 127) + 128);
				else
					LeaderInfo[leader].angle = fish->angle + ((GetRandomControl() & 127) + 128);

				LeaderInfo[leader].angleTime = (GetRandomControl() & 15) + 8;
				LeaderInfo[leader].speedTime = 0;
			}

			if (x < -fishXRange)
			{
				x = -fishXRange;

				if (fish->angle < 1024)
					LeaderInfo[leader].angle = fish->angle - ((GetRandomControl() & 127) + 128);
				else
					LeaderInfo[leader].angle = fish->angle + ((GetRandomControl() & 127) + 128);

				LeaderInfo[leader].angleTime = (GetRandomControl() & 15) + 8;
				LeaderInfo[leader].speedTime = 0;
			}
			else if (x > fishXRange)
			{
				x = fishXRange;

				if (fish->angle < 3072)
					LeaderInfo[leader].angle = fish->angle - ((GetRandomControl() & 127) + 128);
				else
					LeaderInfo[leader].angle = fish->angle + ((GetRandomControl() & 127) + 128);

				LeaderInfo[leader].angleTime = (GetRandomControl() & 15) + 8;
				LeaderInfo[leader].speedTime = 0;
			}

			if ((GetRandomControl() & 15) == 0)
				LeaderInfo[leader].angleTime = 0;

			if (LeaderInfo[leader].angleTime)
				LeaderInfo[leader].angleTime--;
			else
			{
				LeaderInfo[leader].angleTime = (GetRandomControl() & 15) + 8;
				int angAdd = ((GetRandomControl() & 63) + 16) - 8 - 32;

				if ((GetRandomControl() & 3) == 0)
					LeaderInfo[leader].angle += angAdd * 32;
				else
					LeaderInfo[leader].angle += angAdd;

				LeaderInfo[leader].angle &= 4095;
			}

			if (LeaderInfo[leader].speedTime)
				LeaderInfo[leader].speedTime--;
			else
			{
				LeaderInfo[leader].speedTime = (GetRandomControl() & 31) + 32;

				if ((GetRandomControl() & 7) == 0)
					LeaderInfo[leader].speed = (GetRandomControl() & 127) + 128;
				else if ((GetRandomControl() & 3) == 0)
					LeaderInfo[leader].speed += (GetRandomControl() & 127) + 32;
				else if (LeaderInfo[leader].speed > 140)
					LeaderInfo[leader].speed -= (GetRandomControl() & 31) + 48;
				else
				{
					LeaderInfo[leader].speedTime = (GetRandomControl() & 3) + 4;
					LeaderInfo[leader].speed += (GetRandomControl() & 31) - 15;
				}
			}

		}

		int ftx = x;
		int ftz = z;

		fish->x = x;
		fish->z = z;

		fish = (FishInfo*)&Fishes[MAX_FISH + (leader * 24)];

		for (int i = 0; i < 24; i++)
		{
			if (item->Flags & OCB_FISH_LETAL)
			{
				PHD_3DPOS pos;
				pos.Position.x = item->Pose.Position.x + fish->x;
				pos.Position.y = item->Pose.Position.y + fish->y;
				pos.Position.z = item->Pose.Position.z + fish->z;

				if (FishNearLara(&pos, 256, (pirahnaAttack < 2) ? LaraItem : enemy))
				{
					if (PirahnaHitWait == 0)
					{
						DoBloodSplat(item->Pose.Position.x + fish->x, item->Pose.Position.y + fish->y, item->Pose.Position.z + fish->z, 0, 0, (pirahnaAttack < 2) ? LaraItem->RoomNumber : enemy->RoomNumber);
						PirahnaHitWait = 8;
					}

					if (pirahnaAttack != 2)
						DoDamage(LaraItem, PIRAHNA_DAMAGE);
				}
			}

			angle = ((-(mGetAngle(fish->x, fish->z, ftx, ftz) + 0x4000)) / 16) & 4095;
			int dx = fish->x - ftx + ((24 - i) * 128);
			int dz = fish->z - ftz - ((24 - i) * 128);

			dx *= dx;
			dz *= dz;

			deltaAngle = fish->angle - angle;

			if (deltaAngle > 2048)
				deltaAngle -= 4096;
			else if (deltaAngle < -2048)
				deltaAngle += 4096;

			if (deltaAngle > 128)
			{
				fish->angAdd -= 4;
				if (fish->angAdd < -92 - (i / 2))
					fish->angAdd = -92 - (i / 2);
			}
			else if (deltaAngle < -128)
			{
				fish->angAdd += 4;
				if (fish->angAdd > 92 + (i / 2))
					fish->angAdd = 92 + (i / 2);
			}
			else
			{
				fish->angAdd -= fish->angAdd / 4;
				if (abs(fish->angAdd) < 4)
					fish->angAdd = 0;
			}

			fish->angle += fish->angAdd;

			if (deltaAngle > 1024)
				fish->angle += fish->angAdd / 4;
			fish->angle &= 4095;

			if ((dx + dz) < (0x100000 + ((i * 128) * (i * 128))))
			{
				if (fish->speed > 32 + (i * 2))
					fish->speed -= fish->speed / 32;
			}
			else
			{
				if (fish->speed < 160 + (i / 2))
					fish->speed += (GetRandomControl() & 3) + 1 + (i / 2);

				if (fish->speed > 160 + (i / 2) - (i * 4))
					fish->speed = 160 + (i / 2) - (i * 4);
			}

			if (GetRandomControl() & 1)
				fish->speed -= GetRandomControl() & 1;
			else
				fish->speed += GetRandomControl() & 1;

			if (fish->speed < 32)
				fish->speed = 32;
			else if (fish->speed > 200)
				fish->speed = 200;

			fish->swim += (fish->speed / 16) + (fish->speed / 32);
			fish->swim &= 63;

			x = fish->x - fish->speed * phd_sin(fish->angle * 16) / 2;
			z = fish->z + fish->speed * phd_cos(fish->angle * 16) / 2;

			if (z < -32000)
				z = -32000;
			else if (z > 32000)
				z = 32000;
			if (x < -32000)
				x = -32000;
			else if (x > 32000)
				x = 32000;

			fish->x = x;
			fish->z = z;

			if (pirahnaAttack == 0)
			{
				if (abs(fish->y - fish->destY) < 16)
					fish->destY = GetRandomControl() % LeaderInfo[leader].yRange;
			}
			else
			{
				int y = enemy->Pose.Position.y - item->Pose.Position.y;
				if (abs(fish->y - fish->destY) < 16)
					fish->destY = y + (GetRandomControl() & 255);
			}

			fish->y += (fish->destY - fish->y) / 16;
			fish++;
		}
	}

	bool FishNearLara(PHD_3DPOS* pos, int distance, ItemInfo* item)
	{
		int x = pos->Position.x - item->Pose.Position.x;
		int y = abs(pos->Position.y - item->Pose.Position.y);
		int z = pos->Position.z - item->Pose.Position.z;

		if (x < -distance || x > distance || z < -distance || z > distance || y < -SECTOR(3) || y > SECTOR(3))
			return false;

		if ((pow(x, 2) + pow(z, 2)) > pow(distance, 2))
			return false;

		if (y > distance)
			return false;

		return true;
	}
}
