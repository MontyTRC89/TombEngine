#include "framework.h"
#include "Objects/TR3/Entity/tr3_fish_emitter.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Objects/TR3/fish.h"
#include "Specific/level.h"

namespace TEN::Entities::Creatures::TR3
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
		LeaderInfo[shoalNumber].Range.x = (FishRanges[shoalNumber][0] + 2) << 8;
		LeaderInfo[shoalNumber].Range.z = (FishRanges[shoalNumber][1] + 2) << 8;
		LeaderInfo[shoalNumber].Range.y = (FishRanges[shoalNumber][2]) << 8;
	}

	void SetupFish(int leader, ItemInfo* item)
	{
		int fishXRange = LeaderInfo[leader].Range.x;
		int fishYRange = LeaderInfo[leader].Range.y;
		int fishZRange = LeaderInfo[leader].Range.z;

		Fishes[leader].Offset.x = 0;
		Fishes[leader].Offset.y = 0;
		Fishes[leader].Offset.z = 0;
		Fishes[leader].Angle = 0;
		Fishes[leader].Velocity = (GetRandomControl() & 63) + 8;
		Fishes[leader].Swim = GetRandomControl() & 63;

		for (int i = 0; i < 24; i++)
		{
			Fishes[MAX_FISH + (leader * 24) + i].Offset.x = (GetRandomControl() % (fishXRange * 2)) - fishXRange;
			Fishes[MAX_FISH + (leader * 24) + i].Offset.y = (GetRandomControl() % fishYRange);
			Fishes[MAX_FISH + (leader * 24) + i].DestY = (GetRandomControl() % fishYRange);
			Fishes[MAX_FISH + (leader * 24) + i].Offset.z = (GetRandomControl() % (fishZRange * 2)) - fishZRange;
			Fishes[MAX_FISH + (leader * 24) + i].Angle = GetRandomControl() & 4095;
			Fishes[MAX_FISH + (leader * 24) + i].Velocity = (GetRandomControl() & 31) + 32;
			Fishes[MAX_FISH + (leader * 24) + i].Swim = GetRandomControl() & 63;
		}

		LeaderInfo[leader].On = 1;
		LeaderInfo[leader].Angle = 0;
		LeaderInfo[leader].Velocity = (GetRandomControl() & 127) + 32;
		LeaderInfo[leader].AngleTime = 0;
		LeaderInfo[leader].VelocityTime = 0;
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
		if (!LeaderInfo[leader].On)
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

			LeaderInfo[leader].Angle = fish->Angle = 0;// ((-(mGetAngle(fish->Offset.x + item->Pose.Position.x, fish->Offset.z + item->Pose.Position.z, enemy->Pose.Position.x, enemy->Pose.Position.z) + 0x4000)) / 16) & 4095;
			LeaderInfo[leader].Velocity = (GetRandomControl() & 63) + 192;
		}

		int deltaAngle = fish->Angle - LeaderInfo[leader].Angle;

		if (deltaAngle > 2048)
			deltaAngle -= 4096;
		else if (deltaAngle < -2048)
			deltaAngle += 4096;

		if (deltaAngle > 128)
		{
			fish->AngleAdd -= 4;
			if (fish->AngleAdd < -120)
				fish->AngleAdd = -120;
		}
		else if (deltaAngle < -128)
		{
			fish->AngleAdd += 4;
			if (fish->AngleAdd > 120)
				fish->AngleAdd = 120;
		}
		else
		{
			fish->AngleAdd -= fish->AngleAdd / 4;
			if (abs(fish->AngleAdd) < 4)
				fish->AngleAdd = 0;
		}

		fish->Angle += fish->AngleAdd;

		if (deltaAngle > 1024)
			fish->Angle += fish->AngleAdd / 4;
		fish->Angle &= 4095;

		deltaAngle = fish->Velocity - LeaderInfo[leader].Velocity;

		if (deltaAngle < -4)
		{
			deltaAngle = fish->Velocity + (GetRandomControl() & 3) + 1;
			if (deltaAngle < 0)
				deltaAngle = 0;

			fish->Velocity = deltaAngle;
		}
		else if (deltaAngle > 4)
		{
			deltaAngle = fish->Velocity - (GetRandomControl() & 3) - 1;
			if (deltaAngle > 255)
				deltaAngle = 255;

			fish->Velocity = deltaAngle;
		}

		fish->Swim += fish->Velocity / 16;
		fish->Swim &= 63;

		int x = fish->Offset.x;
		int z = fish->Offset.z;

		x -= fish->Velocity * sin(fish->Angle * 16) / 2;
		z += fish->Velocity * cos(fish->Angle * 16) / 2;

		if (pirahnaAttack == 0)
		{
			int fishXRange = LeaderInfo[leader].Range.x;
			int fishZRange = LeaderInfo[leader].Range.z;

			if (z < -fishZRange)
			{
				z = -fishZRange;

				if (fish->Angle < 2048)
					LeaderInfo[leader].Angle = fish->Angle - ((GetRandomControl() & 127) + 128);
				else
					LeaderInfo[leader].Angle = fish->Angle + ((GetRandomControl() & 127) + 128);

				LeaderInfo[leader].AngleTime = (GetRandomControl() & 15) + 8;
				LeaderInfo[leader].VelocityTime = 0;
			}
			else if (z > fishZRange)
			{
				z = fishZRange;

				if (fish->Angle > 3072)
					LeaderInfo[leader].Angle = fish->Angle - ((GetRandomControl() & 127) + 128);
				else
					LeaderInfo[leader].Angle = fish->Angle + ((GetRandomControl() & 127) + 128);

				LeaderInfo[leader].AngleTime = (GetRandomControl() & 15) + 8;
				LeaderInfo[leader].VelocityTime = 0;
			}

			if (x < -fishXRange)
			{
				x = -fishXRange;

				if (fish->Angle < 1024)
					LeaderInfo[leader].Angle = fish->Angle - ((GetRandomControl() & 127) + 128);
				else
					LeaderInfo[leader].Angle = fish->Angle + ((GetRandomControl() & 127) + 128);

				LeaderInfo[leader].AngleTime = (GetRandomControl() & 15) + 8;
				LeaderInfo[leader].VelocityTime = 0;
			}
			else if (x > fishXRange)
			{
				x = fishXRange;

				if (fish->Angle < 3072)
					LeaderInfo[leader].Angle = fish->Angle - ((GetRandomControl() & 127) + 128);
				else
					LeaderInfo[leader].Angle = fish->Angle + ((GetRandomControl() & 127) + 128);

				LeaderInfo[leader].AngleTime = (GetRandomControl() & 15) + 8;
				LeaderInfo[leader].VelocityTime = 0;
			}

			if ((GetRandomControl() & 15) == 0)
				LeaderInfo[leader].AngleTime = 0;

			if (LeaderInfo[leader].AngleTime)
				LeaderInfo[leader].AngleTime--;
			else
			{
				LeaderInfo[leader].AngleTime = (GetRandomControl() & 15) + 8;
				int AngleAdd = ((GetRandomControl() & 63) + 16) - 8 - 32;

				if ((GetRandomControl() & 3) == 0)
					LeaderInfo[leader].Angle += AngleAdd * 32;
				else
					LeaderInfo[leader].Angle += AngleAdd;

				LeaderInfo[leader].Angle &= 4095;
			}

			if (LeaderInfo[leader].VelocityTime)
				LeaderInfo[leader].VelocityTime--;
			else
			{
				LeaderInfo[leader].VelocityTime = (GetRandomControl() & 31) + 32;

				if ((GetRandomControl() & 7) == 0)
					LeaderInfo[leader].Velocity = (GetRandomControl() & 127) + 128;
				else if ((GetRandomControl() & 3) == 0)
					LeaderInfo[leader].Velocity += (GetRandomControl() & 127) + 32;
				else if (LeaderInfo[leader].Velocity > 140)
					LeaderInfo[leader].Velocity -= (GetRandomControl() & 31) + 48;
				else
				{
					LeaderInfo[leader].VelocityTime = (GetRandomControl() & 3) + 4;
					LeaderInfo[leader].Velocity += (GetRandomControl() & 31) - 15;
				}
			}

		}

		int ftx = x;
		int ftz = z;

		fish->Offset.x = x;
		fish->Offset.z = z;

		fish = (FishInfo*)&Fishes[MAX_FISH + (leader * 24)];

		for (int i = 0; i < 24; i++)
		{
			if (item->Flags & OCB_FISH_LETAL)
			{
				PHD_3DPOS pos;
				pos.Position.x = item->Pose.Position.x + fish->Offset.x;
				pos.Position.y = item->Pose.Position.y + fish->Offset.y;
				pos.Position.z = item->Pose.Position.z + fish->Offset.z;

				if (FishNearLara(&pos, 256, (pirahnaAttack < 2) ? LaraItem : enemy))
				{
					if (PirahnaHitWait == 0)
					{
						DoBloodSplat(item->Pose.Position.x + fish->Offset.x, item->Pose.Position.y + fish->Offset.y, item->Pose.Position.z + fish->Offset.z, 0, 0, (pirahnaAttack < 2) ? LaraItem->RoomNumber : enemy->RoomNumber);
						PirahnaHitWait = 8;
					}

					if (pirahnaAttack != 2)
						DoDamage(LaraItem, PIRAHNA_DAMAGE);
				}
			}

			angle = 0;// ((-(mGetAngle(fish->Offset.x, fish->Offset.z, ftx, ftz) + 0x4000)) / 16) & 4095;
			int dx = fish->Offset.x - ftx + ((24 - i) * 128);
			int dz = fish->Offset.z - ftz - ((24 - i) * 128);

			dx *= dx;
			dz *= dz;

			deltaAngle = fish->Angle - angle;

			if (deltaAngle > 2048)
				deltaAngle -= 4096;
			else if (deltaAngle < -2048)
				deltaAngle += 4096;

			if (deltaAngle > 128)
			{
				fish->AngleAdd -= 4;
				if (fish->AngleAdd < -92 - (i / 2))
					fish->AngleAdd = -92 - (i / 2);
			}
			else if (deltaAngle < -128)
			{
				fish->AngleAdd += 4;
				if (fish->AngleAdd > 92 + (i / 2))
					fish->AngleAdd = 92 + (i / 2);
			}
			else
			{
				fish->AngleAdd -= fish->AngleAdd / 4;
				if (abs(fish->AngleAdd) < 4)
					fish->AngleAdd = 0;
			}

			fish->Angle += fish->AngleAdd;

			if (deltaAngle > 1024)
				fish->Angle += fish->AngleAdd / 4;
			fish->Angle &= 4095;

			if ((dx + dz) < (0x100000 + ((i * 128) * (i * 128))))
			{
				if (fish->Velocity > 32 + (i * 2))
					fish->Velocity -= fish->Velocity / 32;
			}
			else
			{
				if (fish->Velocity < 160 + (i / 2))
					fish->Velocity += (GetRandomControl() & 3) + 1 + (i / 2);

				if (fish->Velocity > 160 + (i / 2) - (i * 4))
					fish->Velocity = 160 + (i / 2) - (i * 4);
			}

			if (GetRandomControl() & 1)
				fish->Velocity -= GetRandomControl() & 1;
			else
				fish->Velocity += GetRandomControl() & 1;

			if (fish->Velocity < 32)
				fish->Velocity = 32;
			else if (fish->Velocity > 200)
				fish->Velocity = 200;

			fish->Swim += (fish->Velocity / 16) + (fish->Velocity / 32);
			fish->Swim &= 63;

			x = fish->Offset.x - fish->Velocity * sin(fish->Angle * 16) / 2;
			z = fish->Offset.z + fish->Velocity * cos(fish->Angle * 16) / 2;

			if (z < -32000)
				z = -32000;
			else if (z > 32000)
				z = 32000;
			if (x < -32000)
				x = -32000;
			else if (x > 32000)
				x = 32000;

			fish->Offset.x = x;
			fish->Offset.z = z;

			if (pirahnaAttack == 0)
			{
				if (abs(fish->Offset.y - fish->DestY) < 16)
					fish->DestY = GetRandomControl() % LeaderInfo[leader].Range.y;
			}
			else
			{
				int y = enemy->Pose.Position.y - item->Pose.Position.y;
				if (abs(fish->Offset.y - fish->DestY) < 16)
					fish->DestY = y + (GetRandomControl() & 255);
			}

			fish->Offset.y += (fish->DestY - fish->Offset.y) / 16;
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
