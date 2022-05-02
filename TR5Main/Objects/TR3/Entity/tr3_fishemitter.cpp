#include "framework.h"
#include "Objects/TR3/Entity/tr3_fishemitter.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Objects/TR3/fish.h"
#include "Specific/level.h"

int PirahnaHitWait = false;
int CarcassItem = NO_ITEM;

#define PIRAHNA_DAMAGE 4
#define X 0
#define Y 1
#define Z 2
#define XYZ	3
#define MAX_FISH 8
#define OCB_FISH_LETHAL 0x1000
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

void SetupFish(int leader, ITEM_INFO* item)
{
	auto range = LeaderInfo[leader].Range;

	Fishes[leader].Offset = Vector3Int();
	Fishes[leader].Angle = 0;
	Fishes[leader].Velocity = (GetRandomControl() & 63) + 8;
	Fishes[leader].Swim = GetRandomControl() & 63;

	for (int i = 0; i < 24; i++)
	{
		Fishes[MAX_FISH + (leader * 24) + i].Offset.x = (GetRandomControl() % (range.x * 2)) - range.z;
		Fishes[MAX_FISH + (leader * 24) + i].Offset.y = (GetRandomControl() % range.y);
		Fishes[MAX_FISH + (leader * 24) + i].DestY = (GetRandomControl() % range.y);
		Fishes[MAX_FISH + (leader * 24) + i].Offset.z = (GetRandomControl() % (range.z * 2)) - range.z;
		Fishes[MAX_FISH + (leader * 24) + i].Angle = GetRandomControl() & 4095;
		Fishes[MAX_FISH + (leader * 24) + i].Velocity = (GetRandomControl() & 31) + 32;
		Fishes[MAX_FISH + (leader * 24) + i].Swim = GetRandomControl() & 63;
	}

	LeaderInfo[leader].On = true;
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
	float angle = 0;

	int leader = item->HitPoints;
	if (!LeaderInfo[leader].On)
		SetupFish(leader, item);

	if (item->TriggerFlags & OCB_FISH_LETHAL)  
	{
		if ((item->TriggerFlags == OCB_FISH_EAT_CARCASS) == 0)
			pirahnaAttack = LaraItem->RoomNumber == item->RoomNumber;
		else
		{
			if (CarcassItem != -1)
				pirahnaAttack = 2;
			else
				pirahnaAttack = LaraItem->RoomNumber == item->RoomNumber;
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

		LeaderInfo[leader].Angle = fish->Angle = Angle::ShrtToRad(((-(Angle::RadToShrt(mGetAngle(fish->Offset.x + item->Pose.Position.x, fish->Offset.z + item->Pose.Position.z, enemy->Pose.Position.x, enemy->Pose.Position.z)) + Angle::DegToShrt(90.0f))) / 16) & 4095); // TODO
		LeaderInfo[leader].Velocity = (GetRandomControl() & 63) + 192;
	}

	float deltaAngle = Angle::ShortestAngle(fish->Angle, LeaderInfo[leader].Angle);

	if (deltaAngle > Angle::DegToRad(11.35f))
		deltaAngle -= Angle::DegToRad(22.5f);
	else if (deltaAngle < Angle::DegToRad(-11.35f))
		deltaAngle += Angle::DegToRad(22.5f);

	if (deltaAngle > Angle::DegToRad(0.7f))
	{
		fish->AngleAdd -= Angle::DegToRad(0.02f);
		if (fish->AngleAdd < Angle::DegToRad(-0.7f))
			fish->AngleAdd = Angle::DegToRad(-0.7f);
	}
	else if (deltaAngle < -Angle::DegToRad(-0.7f))
	{
		fish->AngleAdd += Angle::DegToRad(0.02f);
		if (fish->AngleAdd > Angle::DegToRad(0.7f))
			fish->AngleAdd = Angle::DegToRad(0.7f);
	}
	else
	{
		fish->AngleAdd -= fish->AngleAdd / 4;
		if (abs(fish->AngleAdd) < Angle::DegToRad(0.02f))
			fish->AngleAdd = 0;
	}

	fish->Angle += fish->AngleAdd;

	if (deltaAngle > Angle::DegToRad(5.6f))
		fish->Angle += fish->AngleAdd / 4;
	fish->Angle = Angle::ShrtToRad(Angle::RadToShrt(fish->Angle) & 4095); // TODO

	deltaAngle = fish->Velocity - LeaderInfo[leader].Velocity;

	if (deltaAngle < Angle::DegToRad(-0.02f))
	{
		deltaAngle = fish->Velocity + (GetRandomControl() & 3) + 1;
		if (deltaAngle < 0)
			deltaAngle = 0;

		fish->Velocity = deltaAngle;
	}
	else if (deltaAngle > Angle::DegToRad(0.02f))
	{
		deltaAngle = fish->Velocity - (GetRandomControl() & 3) - 1;
		if (deltaAngle > Angle::DegToRad(1.4f))
			deltaAngle = Angle::DegToRad(1.4f);

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

			if (fish->Angle < Angle::DegToRad(11.25f))
				LeaderInfo[leader].Angle = fish->Angle - Angle::ShrtToRad(((GetRandomControl() & 127) + 128)); // TODO
			else
				LeaderInfo[leader].Angle = fish->Angle + Angle::ShrtToRad(((GetRandomControl() & 127) + 128)); // TODO

			LeaderInfo[leader].AngleTime = (GetRandomControl() & 15) + 8;
			LeaderInfo[leader].VelocityTime = 0;
		}
		else if (z > fishZRange)
		{
			z = fishZRange;

			if (fish->Angle > Angle::DegToRad(16.9f))
				LeaderInfo[leader].Angle = fish->Angle - Angle::ShrtToRad(((GetRandomControl() & 127) + 128)); // TODO
			else
				LeaderInfo[leader].Angle = fish->Angle + Angle::ShrtToRad(((GetRandomControl() & 127) + 128)); // TODO

			LeaderInfo[leader].AngleTime = (GetRandomControl() & 15) + 8;
			LeaderInfo[leader].VelocityTime = 0;
		}

		if (x < -fishXRange)
		{
			x = -fishXRange;

			if (fish->Angle < Angle::DegToRad(5.6f))
				LeaderInfo[leader].Angle = fish->Angle - Angle::ShrtToRad(((GetRandomControl() & 127) + 128)); // TODO
			else
				LeaderInfo[leader].Angle = fish->Angle + Angle::ShrtToRad(((GetRandomControl() & 127) + 128)); // TODO

			LeaderInfo[leader].AngleTime = (GetRandomControl() & 15) + 8;
			LeaderInfo[leader].VelocityTime = 0;
		}
		else if (x > fishXRange)
		{
			x = fishXRange;

			if (fish->Angle < Angle::DegToRad(16.9f))
				LeaderInfo[leader].Angle = fish->Angle - Angle::ShrtToRad(((GetRandomControl() & 127) + 128)); // TODO
			else
				LeaderInfo[leader].Angle = fish->Angle + Angle::ShrtToRad(((GetRandomControl() & 127) + 128)); // TODO

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
			float AngleAdd = ((GetRandomControl() & 63) + 16) - 8 - 32;

			if ((GetRandomControl() & 3) == 0)
				LeaderInfo[leader].Angle += AngleAdd * 32;
			else
				LeaderInfo[leader].Angle += AngleAdd;

			LeaderInfo[leader].Angle = Angle::ShrtToRad(Angle::RadToShrt(LeaderInfo[leader].Angle) & 4095);
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
		if (item->Flags & OCB_FISH_LETHAL)
		{
			PoseData pos;
			pos.Position = item->Pose.Position + fish->Offset;

			if (FishNearLara(&pos, 256, (pirahnaAttack < 2) ? LaraItem : enemy))
			{
				if (PirahnaHitWait == 0)
				{
					DoBloodSplat(item->Pose.Position.x + fish->Offset.x, item->Pose.Position.y + fish->Offset.y, item->Pose.Position.z + fish->Offset.z, 0, 0, (pirahnaAttack < 2) ? LaraItem->RoomNumber : enemy->RoomNumber);
					PirahnaHitWait = 8;
				}

				if (pirahnaAttack != 2)
					LaraItem->HitPoints -= PIRAHNA_DAMAGE;
			}
		}

		angle = Angle::ShrtToRad(((-(Angle::RadToShrt(mGetAngle(fish->Offset.x, fish->Offset.z, ftx, ftz)) + Angle::DegToShrt(90.0f))) / 16) & 4095);
		int dx = fish->Offset.x - ftx + ((24 - i) * CLICK(0.5f));
		int dz = fish->Offset.z - ftz - ((24 - i) * CLICK(0.5f));

		dx *= dx;
		dz *= dz;

		deltaAngle = fish->Angle - angle;

		if (deltaAngle > Angle::DegToRad(11.25f))
			deltaAngle -= Angle::DegToRad(22.5f);
		else if (deltaAngle < Angle::DegToRad(-11.25f))
			deltaAngle += Angle::DegToRad(22.5f);

		if (deltaAngle > Angle::DegToRad(0.7f))
		{
			fish->AngleAdd -= Angle::DegToRad(0.02f);
			if (fish->AngleAdd < Angle::ShrtToRad(-92 - (i / 2))) // TODO
				fish->AngleAdd = Angle::ShrtToRad(-92 - (i / 2));
		}
		else if (deltaAngle < Angle::DegToRad(-0.7f))
		{
			fish->AngleAdd += Angle::DegToRad(0.02f);
			if (fish->AngleAdd > Angle::ShrtToRad(92 + (i / 2))) // TODO
				fish->AngleAdd = Angle::ShrtToRad(92 + (i / 2));
		}
		else
		{
			fish->AngleAdd -= fish->AngleAdd / 4;
			if (abs(fish->AngleAdd) < Angle::DegToRad(0.02f))
				fish->AngleAdd = 0;
		}

		fish->Angle += fish->AngleAdd;

		if (deltaAngle > Angle::DegToRad(5.6f))
			fish->Angle += fish->AngleAdd / 4;
		fish->Angle = Angle::ShrtToRad(Angle::RadToShrt(fish->Angle) & 4095);

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

		if (z < -SECTOR(31.25f))
			z = -SECTOR(31.25f);
		else if (z > SECTOR(31.25f))
			z = SECTOR(31.25f);
		if (x < -SECTOR(31.25f))
			x = -SECTOR(31.25f);
		else if (x > SECTOR(31.25f))
			x = SECTOR(31.25f);

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

bool FishNearLara(PoseData* pos, int distance, ITEM_INFO* item)
{
	int x = pos->Position.x - item->Pose.Position.x;
	int y = abs(pos->Position.y - item->Pose.Position.y);
	int z = pos->Position.z - item->Pose.Position.z;

	if (x < -distance || x > distance ||
		y < -SECTOR(3) || y > SECTOR(3) ||
		z < -distance || z > distance)
	{
		return false;
	}

	if ((pow(x, 2) + pow(z, 2)) > pow(distance, 2))
		return false;

	if (y > distance)
		return false;

	return true;
}
