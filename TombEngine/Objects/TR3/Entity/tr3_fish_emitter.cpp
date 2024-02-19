#include "framework.h"
#include "Objects/TR3/Entity/tr3_fish_emitter.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Objects/TR3/fish.h"
#include "Specific/level.h"
#include "Renderer/Renderer.h"

using namespace TEN::Math;
using namespace TEN::Renderer;

namespace TEN::Entities::Creatures::TR3
{
	int PirahnaHitWait = false;
	int CarcassItem = NO_ITEM;

	#define PIRAHNA_DAMAGE 4
	#define X 0
	#define Y 1
	#define Z 2
	#define XYZ	3
	#define OCB_FISH_LETAL 0x1000
	#define OCB_FISH_EAT_CARCASS 0x2000

	constexpr auto MAX_FISH = 8;

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

	void SetupShoal(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		int shoalNumber = 0 & 7;
		item->HitPoints = shoalNumber;

		LeaderInfo[shoalNumber].xRange = (FishRanges[shoalNumber][0] + 2) << 8;
		LeaderInfo[shoalNumber].yRange = (FishRanges[shoalNumber][2]) << 8;
		LeaderInfo[shoalNumber].zRange = (FishRanges[shoalNumber][1] + 2) << 8;
	}

	void SetupFish(int leader, ItemInfo* item)
	{



		FishLeaderInfo* pLeader = &LeaderInfo[leader];
		FishInfo* pFish = &Fishes[leader];

		int fishXRange = pLeader->xRange;
		int fishYRange = pLeader->yRange;
		int fishZRange = pLeader->zRange;

		pFish->Pose.Position.x = 0;
		pFish->Pose.Position.y = 0;
		pFish->Pose.Position.z = 0;
		pFish->angle = 0;
		pFish->speed = (GetRandomControl() & 63) + 8;
		pFish->swim = GetRandomControl() & 63;

		for (int i = 0; i < 24; i++)
		{
			pFish = &Fishes[(leader * 24) + 8 + i];
			pFish->Pose.Position.x = (GetRandomControl() % (fishXRange << 1)) - fishXRange;
			pFish->Pose.Position.y = (GetRandomControl() % fishYRange);
			pFish->Pose.Position.z = (GetRandomControl() % (fishZRange << 1)) - fishZRange;
			pFish->destY = (GetRandomControl() % fishYRange);
			pFish->angle = GetRandomControl() & 4095;
			pFish->speed = (GetRandomControl() & 31) + 32;
			pFish->swim = GetRandomControl() & 63;			
		}

		pLeader->on = 1;
		pLeader->angle = 0;
		pLeader->speed = (GetRandomControl() & 127) + 32;
		pLeader->angleTime = 0;
		pLeader->speedTime = 0;
	}

	void ControlFish(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* enemy = item;

		if (!TriggerActive(item))
			return;

		int pirahnaAttack = 0;
		int angle = 0;

		item->HitPoints = 0 & 7;

		int leader = item->HitPoints;

		FishInfo* fish = &Fishes[leader];
		FishLeaderInfo* pLeader = &LeaderInfo[leader];

		if (!pLeader->on)
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



		if (pirahnaAttack)
		{
			if (pirahnaAttack == 1)
				enemy = LaraItem;
			else
				enemy = &g_Level.Items[CarcassItem];

			pLeader->angle = fish->angle = ((-(Geometry::GetOrientToPoint(Vector3(fish->Pose.Position.x + item->Pose.Position.x, 0.0f, fish->Pose.Position.z + item->Pose.Position.z), enemy->Pose.Position.ToVector3()).y + ANGLE(90.0f))) / 16) & ANGLE(22.5f);
			pLeader->speed = (Random::GenerateInt() & 63) -64;
		}

		int deltaAngle = fish->angle - pLeader->angle;

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
			deltaAngle = fish->speed + (Random::GenerateInt() & 3) + 1;
			if (deltaAngle < 0)
				deltaAngle = 0;

			fish->speed = deltaAngle;
		}
		else if (deltaAngle > 4)
		{
			deltaAngle = fish->speed - (Random::GenerateInt() & 3) - 1;
			if (deltaAngle > 255)
				deltaAngle = 255;

			fish->speed = deltaAngle;
		}

		fish->swim = (fish->swim + (fish->speed >> 4)) & 0x3F;;

		
		int x = fish->Pose.Position.x - (fish->speed * phd_sin(fish->angle));
		int z = fish->Pose.Position.z + (fish->speed * phd_cos(fish->angle));

		x -= fish->speed * phd_sin(fish->angle * 16) / 2;
		z += fish->speed * phd_cos(fish->angle * 16) / 2;

		if (pirahnaAttack == 0)
		{
			int fishXRange = pLeader->xRange;
			int fishZRange = pLeader->zRange;

			if (z < -fishZRange)
			{
				z = -fishZRange;

				if (fish->angle < 2048)
					pLeader->angle = fish->angle - ((Random::GenerateInt() & 127) - 128);
				else
					pLeader->angle = fish->angle + ((Random::GenerateInt() & 127) + 128);

				pLeader->angleTime = (Random::GenerateInt() & 15) + 8;
				pLeader->speedTime = 0;
			}
			else if (z > fishZRange)
			{
				z = fishZRange;

				if (fish->angle > 3072)
					pLeader->angle = fish->angle - ((Random::GenerateInt() & 127) - 128);
				else
					pLeader->angle = fish->angle + ((Random::GenerateInt() & 127) + 128);

				pLeader->angleTime = (Random::GenerateInt() & 15) + 8;
				pLeader->speedTime = 0;
			}

			if (x < -fishXRange)
			{
				x = -fishXRange;

				if (fish->angle < 1024)
					pLeader->angle = fish->angle - ((Random::GenerateInt() & 127) - 128);
				else
					pLeader->angle = fish->angle + ((Random::GenerateInt() & 127) + 128);

				pLeader->angleTime = (Random::GenerateInt() & 15) + 8;
				pLeader->speedTime = 0;
			}
			else if (x > fishXRange)
			{
				x = fishXRange;

				if (fish->angle < 3072)
					pLeader->angle = fish->angle - ((Random::GenerateInt() & 127) - 128);
				else
					pLeader->angle = fish->angle + ((Random::GenerateInt() & 127) + 128);

				pLeader->angleTime = (Random::GenerateInt() & 15) + 8;
				pLeader->speedTime = 0;
			}

			if ((Random::GenerateInt() & 15) == 0)
				pLeader->angleTime = 0;

			if (pLeader->angleTime)
				pLeader->angleTime--;
			else
			{
				pLeader->angleTime = (Random::GenerateInt() & 15) + 8;
				int angAdd = (GetRandomControl() & 0x3F) - 24;

				if ((Random::GenerateInt() & 3) == 0)
					angAdd <<= 5;

				pLeader->angle = (pLeader->angle + angAdd) & 0xFFF;
			}

			if (pLeader->speedTime)
				pLeader->speedTime--;
			else
			{
				pLeader->speedTime = (Random::GenerateInt() & 31) + 32;

				if ((Random::GenerateInt() & 7) == 0)
					pLeader->speed = (Random::GenerateInt() & 127) + 128;
				else if ((Random::GenerateInt() & 3) == 0)
					pLeader->speed += (Random::GenerateInt() & 127) + 32;
				else if (pLeader->speed > 140)
					pLeader->speed += 208 - (GetRandomControl() & 0x1F);
				else
				{
					pLeader->speedTime = (Random::GenerateInt() & 3) + 4;
					pLeader->speed += (Random::GenerateInt() & 31) - 15;
				}
			}

		}

		int ftx = x;
		int ftz = z;

		fish->Pose.Position.x = (short)x;
		fish->Pose.Position.z = (short)z;

	

		for (int i = 0; i < 24; i++)
		{
			fish = &Fishes[MAX_FISH * leader + i + 8];

			if (item->Flags & OCB_FISH_LETAL)
			{
				Pose pos;
				pos.Position.x = item->Pose.Position.x + fish->Pose.Position.x;
				pos.Position.y = item->Pose.Position.y + fish->Pose.Position.y;
				pos.Position.z = item->Pose.Position.z + fish->Pose.Position.z;

				g_Renderer.AddDebugSphere(Vector3(fish->Pose.Position.x, fish->Pose.Position.y, fish->Pose.Position.z), 46, Vector4(1, 0, 0, 1), RendererDebugPage::None);
				g_Renderer.AddDebugLine(Vector3(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z), Vector3(fish->Pose.Position.x, fish->Pose.Position.y, fish->Pose.Position.z), Vector4(1, 0, 0, 1), RendererDebugPage::None);

				if (FishNearLara(&pos, 256, (pirahnaAttack < 2) ? LaraItem : enemy))
				{
					if (PirahnaHitWait == 0)
					{
						DoBloodSplat(item->Pose.Position.x + fish->Pose.Position.x, item->Pose.Position.y + fish->Pose.Position.y, item->Pose.Position.z + fish->Pose.Position.z, 0, 0, (pirahnaAttack < 2) ? LaraItem->RoomNumber : enemy->RoomNumber);
						PirahnaHitWait = 8;
					}

					if (pirahnaAttack != 2)
						DoDamage(LaraItem, PIRAHNA_DAMAGE);
				}
			}

			angle = ((-(Geometry::GetOrientToPoint(Vector3(fish->Pose.Position.x, 0.0f, fish->Pose.Position.z), Vector3(ftx, 0.0f, ftz)).y + ANGLE(90.0f))) / 16) & ANGLE(22.5f);
			int dx = SQUARE(fish->Pose.Position.x - ftx - 128 * i + 3072);
			int dz = SQUARE(fish->Pose.Position.z - ftz + 128 * i - 3072);

			
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

			if (dx + dz < 16384 * SQUARE(i) + 1048576)
			{
				if (fish->speed > 2 * i + 32)
					fish->speed -= fish->speed / 32;
			}
			else
			{
				if (fish->speed < 160 + (i / 2))
					fish->speed += ((Random::GenerateInt() & 3) + 1) + (i / 2);

				if (fish->speed > 160 + (i / 2) - (i * 4))
					fish->speed = 160 + (i / 2) - (i * 4);
			}

			if (Random::GenerateInt() & 1)
				fish->speed -= Random::GenerateInt() & 1;
			else
				fish->speed += Random::GenerateInt() & 1;

			if (fish->speed < 32)
				fish->speed = 32;
			else if (fish->speed > 200)
				fish->speed = 200;

			fish->swim = (fish->swim + (fish->speed >> 4) + (fish->speed >> 5)) & 0x3F;
		
			x = fish->Pose.Position.x - fish->speed * phd_sin(fish->angle * 16) / 2;
			z = fish->Pose.Position.z + fish->speed * phd_cos(fish->angle * 16) / 2;

			if (z < -32000)
				z = -32000;
			else if (z > 32000)
				z = 32000;
			if (x < -32000)
				x = -32000;
			else if (x > 32000)
				x = 32000;

			fish->Pose.Position.x = (short)x;
			fish->Pose.Position.z = (short)z;

		

			if (pirahnaAttack == 0)
			{
				if (abs(fish->Pose.Position.y - fish->destY) < 16)
					fish->destY = Random::GenerateInt() % LeaderInfo[leader].yRange;
			}
			else
			{
				int y = enemy->Pose.Position.y - item->Pose.Position.y;
				if (abs(fish->Pose.Position.y - fish->destY) < 16)
					fish->destY = y + (Random::GenerateInt() & 255);
			}

			fish->Pose.Position.y += (fish->destY - fish->Pose.Position.y) / 16;
			//fish++;
		}

		

	}

	bool FishNearLara(Pose* pos, int distance, ItemInfo* item)
	{
		int x = pos->Position.x - item->Pose.Position.x;
		int y = abs(pos->Position.y - item->Pose.Position.y);
		int z = pos->Position.z - item->Pose.Position.z;

		if (x < -distance || x > distance || z < -distance || z > distance || y < -BLOCK(3) || y > BLOCK(3))
			return false;

		if ((pow(x, 2) + pow(z, 2)) > pow(distance, 2))
			return false;

		if (y > distance)
			return false;

		return true;
	}
}
