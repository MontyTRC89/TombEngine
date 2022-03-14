#include "framework.h"
#include "Objects/TR4/Entity/tr4_beetle_swarm.h"

#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Specific/trmath.h"
#include "Game/Lara/lara.h"
#include "Specific/setup.h"
#include "Game/control/flipeffect.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	BeetleInfo BeetleSwarm[NUM_BEETLES];
	int NextBeetle;

	void InitialiseBeetleSwarm(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->ItemFlags[0] = (item->TriggerFlags / 1000) & 1;
		item->ItemFlags[1] = (item->TriggerFlags / 1000) & 2;
		item->ItemFlags[2] = (item->TriggerFlags / 1000) & 4;

		item->TriggerFlags = item->TriggerFlags % 1000;

		if (!item->ItemFlags[1])
		{
			if (item->Position.yRot <= ANGLE(22.5f) || item->Position.yRot >= ANGLE(157.5f))
			{
				if (!(item->Position.yRot >= -ANGLE(22.5f) || item->Position.yRot <= -ANGLE(157.5f)))
					item->Position.xPos += CLICK(2);
			}
			else
				item->Position.xPos -= CLICK(2);

			if (item->Position.yRot <= -ANGLE(45.0f) || item->Position.yRot >= ANGLE(45.0f))
			{
				if (item->Position.yRot < -ANGLE(112.5f) || item->Position.yRot > ANGLE(112.5f))
					item->Position.zPos += CLICK(2);
			}
			else
				item->Position.zPos -= CLICK(2);
		}
	}

	void BeetleSwarmControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->TriggerFlags)
		{
			if (!item->ItemFlags[2] || !(GetRandomControl() & 0xF))
			{
				item->TriggerFlags--;
				if (item->ItemFlags[2])
				{
					if (GetRandomControl() & 1)
						item->ItemFlags[2]--;
				}

				short beetleNumber = GetFreeBeetle();
				if (beetleNumber != NO_ITEM)
				{
					auto* beetle = &BeetleSwarm[beetleNumber];

					beetle->Position.xPos = item->Position.xPos;
					beetle->Position.yPos = item->Position.yPos;
					beetle->Position.zPos = item->Position.zPos;
					beetle->RoomNumber = item->RoomNumber;

					if (item->ItemFlags[0])
					{
						beetle->Position.yRot = 2 * GetRandomControl();
						beetle->VerticalVelocity= -16 - (GetRandomControl() & 0x1F);
					}
					else
					{
						beetle->Position.yRot = item->Position.yRot + (GetRandomControl() & 0x3FFF) - ANGLE(45.0f);
						beetle->VerticalVelocity = 0;
					}

					beetle->Position.xRot = 0;
					beetle->Position.zRot = 0;
					beetle->On = true;
					beetle->Flags = 0;
					beetle->Velocity = (GetRandomControl() & 0x1F) + 1;
				}
			}
		}
	}

	void ClearBeetleSwarm()
	{
		if (Objects[ID_LITTLE_BEETLE].loaded)
		{
			ZeroMemory(BeetleSwarm, NUM_BEETLES * sizeof(BeetleInfo));
			NextBeetle = 0;
			FlipEffect = -1;
		}
	}

	short GetFreeBeetle()
	{
		BeetleInfo* beetle = &BeetleSwarm[NextBeetle];
		short result = NextBeetle;

		int i = 0;
		while (beetle->On)
		{
			if (result == NUM_BEETLES - 1)
			{
				beetle = &BeetleSwarm[0];
				result = 0;
			}
			else
			{
				result++;
				beetle++;
			}

			if (++i >= NUM_BEETLES)
				return NO_ITEM;
		}

		NextBeetle = (result + 1) & (NUM_BEETLES - 1);

		return result;
	}

	void UpdateBeetleSwarm()
	{
		for (int i = 0; i < NUM_BEETLES; i++)
		{
			auto* beetle = &BeetleSwarm[i];

			if (beetle->On)
			{
				int oldx = beetle->Position.xPos;
				int oldy = beetle->Position.yPos;
				int oldz = beetle->Position.zPos;

				beetle->Position.xPos += beetle->Velocity * phd_sin(beetle->Position.yRot);
				beetle->Position.yPos += beetle->VerticalVelocity;
				beetle->Position.zPos += beetle->Velocity * phd_cos(beetle->Position.yRot);

				beetle->VerticalVelocity += GRAVITY;

				int dx = LaraItem->Position.xPos - beetle->Position.xPos;
				int dy = LaraItem->Position.yPos - beetle->Position.yPos;
				int dz = LaraItem->Position.zPos - beetle->Position.zPos;

				short angle = phd_atan(dz, dx) - beetle->Position.yRot;

				if (abs(dx) < 85 &&
					abs(dy) < 85 &&
					abs(dz) < 85)
				{
					LaraItem->HitPoints--;
					LaraItem->HitStatus = true;
				}

				if (beetle->Flags)
				{
					if (abs(dx) + abs(dz) <= SECTOR(1))
					{
						if (beetle->Velocity & 1)
							beetle->Position.yRot += ANGLE(2.8f);
						else
							beetle->Position.yRot -= ANGLE(2.8f);

						beetle->Velocity = 48 - Lara.LitTorch * 64 - (abs(angle) / 128);
						if (beetle->Velocity < -16)
							beetle->Velocity = i & 0xF;
					}
					else
					{
						if (beetle->Velocity < (i & 0x1F) + 24)
							beetle->Velocity++;
						
						if (abs(angle) >= ANGLE(22.5f))
						{
							if (angle >= 0)
								beetle->Position.yRot += ANGLE(5.6f);
							else
								beetle->Position.yRot -= ANGLE(5.6f);
						}
						else
							beetle->Position.yRot += 8 * (Wibble - i);
					}
				}

				FLOOR_INFO* floor = GetFloor(beetle->Position.xPos, beetle->Position.yPos, beetle->Position.zPos, &beetle->RoomNumber);
				int height = GetFloorHeight(floor, beetle->Position.xPos, beetle->Position.yPos, beetle->Position.zPos);
				if (height < (beetle->Position.yPos - SECTOR(1.25f)) || height == NO_HEIGHT)
				{
					// Beetle has hit a wall a high step.
					if (angle <= 0)
						beetle->Position.yRot -= ANGLE(90.0f);
					else
						beetle->Position.yRot += ANGLE(90.0f);

					beetle->Position.xPos = oldx;
					beetle->Position.yPos = oldy;
					beetle->Position.zPos = oldz;
					beetle->Position.xRot = 0;
					beetle->Position.zRot = 0;
					beetle->VerticalVelocity = 0;
				}
				else
				{
					// Beetle is below the floor.
					if (beetle->Position.yPos > height)
					{
						beetle->Position.yPos = height;
						beetle->Position.xRot = 0;
						beetle->Position.zRot = 0;
						beetle->VerticalVelocity = 0;
						beetle->Flags = 1;
					}
				}

				if (beetle->VerticalVelocity >= 500)
				{
					beetle->On = false;
					NextBeetle = 0;
				}
				else
					beetle->Position.xRot = -64 * beetle->VerticalVelocity;
			}
		}
	}
}
