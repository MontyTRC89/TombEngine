#include "framework.h"
#include "Objects/TR4/Entity/tr4_littlebeetle.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Specific/trmath.h"
#include "Game/Lara/lara.h"
#include "Specific/setup.h"
#include "Game/control/flipeffect.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	SCARAB_STRUCT Scarabs[NUM_SCARABS];
	int NextScarab;

	void InitialiseScarabs(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		item->itemFlags[0] = (item->triggerFlags / 1000) & 1;
		item->itemFlags[1] = (item->triggerFlags / 1000) & 2;
		item->itemFlags[2] = (item->triggerFlags / 1000) & 4;

		item->triggerFlags = item->triggerFlags % 1000;

		if (!item->itemFlags[1])
		{
			if (item->pos.yRot <= 4096 || item->pos.yRot >= 28672)
			{
				if (!(item->pos.yRot >= -4096 || item->pos.yRot <= -28672))
					item->pos.xPos += 512;
			}
			else
			{
				item->pos.xPos -= 512;
			}

			if (item->pos.yRot <= -8192 || item->pos.yRot >= 0x2000)
			{
				if (item->pos.yRot < -20480 || item->pos.yRot > 20480)
				{
					item->pos.zPos += 512;
				}
			}
			else
			{
				item->pos.zPos -= 512;
			}
		}
	}

	void ScarabsControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->triggerFlags)
		{
			if (!item->itemFlags[2] || !(GetRandomControl() & 0xF))
			{
				item->triggerFlags--;
				if (item->itemFlags[2])
				{
					if (GetRandomControl() & 1)
					{
						item->itemFlags[2]--;
					}
				}

				short beetleNum = GetFreeScarab();
				if (beetleNum != NO_ITEM)
				{
					SCARAB_STRUCT* beetle = &Scarabs[beetleNum];
					beetle->pos.xPos = item->pos.xPos;
					beetle->pos.yPos = item->pos.yPos;
					beetle->pos.zPos = item->pos.zPos;
					beetle->roomNumber = item->roomNumber;
					if (item->itemFlags[0])
					{
						beetle->pos.yRot = 2 * GetRandomControl();
						beetle->fallspeed= -16 - (GetRandomControl() & 0x1F);
					}
					else
					{
						beetle->pos.yRot = item->pos.yRot + (GetRandomControl() & 0x3FFF) - ANGLE(45);
						beetle->fallspeed = 0;
					}
					beetle->pos.xRot = 0;
					beetle->pos.zRot = 0;
					beetle->on = true;
					beetle->flags = 0;
					beetle->speed = (GetRandomControl() & 0x1F) + 1;
				}
			}
		}
	}

	void ClearScarabs()
	{
		if (Objects[ID_LITTLE_BEETLE].loaded)
		{
			ZeroMemory(Scarabs, NUM_SCARABS * sizeof(SCARAB_STRUCT));
			NextScarab = 0;
			FlipEffect = -1;
		}
	}

	short GetFreeScarab()
	{
		short result = NextScarab;
		SCARAB_STRUCT* beetle = &Scarabs[NextScarab];

		int i = 0;
		while (beetle->on)
		{
			if (result == NUM_SCARABS - 1)
			{
				beetle = &Scarabs[0];
				result = 0;
			}
			else
			{
				result++;
				beetle++;
			}
			if (++i >= NUM_SCARABS)
			{
				return NO_ITEM;
			}
		}

		NextScarab = (result + 1) & (NUM_SCARABS - 1);

		return result;
	}

	void UpdateScarabs()
	{
		for (int i = 0; i < NUM_SCARABS; i++)
		{
			SCARAB_STRUCT* beetle = &Scarabs[i];

			if (beetle->on)
			{
				int oldx = beetle->pos.xPos;
				int oldy = beetle->pos.yPos;
				int oldz = beetle->pos.zPos;

				beetle->pos.xPos += beetle->speed * phd_sin(beetle->pos.yRot);
				beetle->pos.yPos += beetle->fallspeed;
				beetle->pos.zPos += beetle->speed * phd_cos(beetle->pos.yRot);

				beetle->fallspeed += GRAVITY;

				int dx = LaraItem->pos.xPos - beetle->pos.xPos;
				int dy = LaraItem->pos.yPos - beetle->pos.yPos;
				int dz = LaraItem->pos.zPos - beetle->pos.zPos;

				short angle = phd_atan(dz, dx) - beetle->pos.yRot;

				if (abs(dx) < 85 && abs(dy) < 85 && abs(dz) < 85)
				{
					LaraItem->hitPoints--;
					LaraItem->hitStatus = true;
				}

				if (beetle->flags)
				{
					if (abs(dx) + abs(dz) <= 1024)
					{
						if (beetle->speed & 1)
							beetle->pos.yRot += 512;
						else
							beetle->pos.yRot -= 512;

						beetle->speed = 48 - Lara.litTorch * 64 - (abs(angle) / 128);
						if (beetle->speed < -16)
						{
							beetle->speed = i & 0xF;
						}
					}
					else
					{
						if (beetle->speed < (i & 0x1F) + 24)
						{
							beetle->speed++;
						}
						if (abs(angle) >= 4096)
						{
							if (angle >= 0)
								beetle->pos.yRot += 1024;
							else
								beetle->pos.yRot -= 1024;
						}
						else
						{
							beetle->pos.yRot += 8 * (Wibble - i);
						}
					}
				}

				FLOOR_INFO* floor = GetFloor(beetle->pos.xPos, beetle->pos.yPos, beetle->pos.zPos, &beetle->roomNumber);
				int height = GetFloorHeight(floor, beetle->pos.xPos, beetle->pos.yPos, beetle->pos.zPos);
				if (height < beetle->pos.yPos - 1280 || height == NO_HEIGHT)
				{
					// If beetle has hit a wall or a too high step
					if (angle <= 0)
						beetle->pos.yRot -= ANGLE(90);
					else
						beetle->pos.yRot += ANGLE(90);

					beetle->pos.xPos = oldx;
					beetle->pos.yPos = oldy;
					beetle->pos.zPos = oldz;
					beetle->pos.xRot = 0;
					beetle->pos.zRot = 0;
					beetle->fallspeed = 0;
				}
				else
				{
					// If beetle is below the floor
					if (beetle->pos.yPos > height)
					{
						beetle->pos.yPos = height;
						beetle->flags = 1;
						beetle->fallspeed = 0;
						beetle->pos.xRot = 0;
						beetle->pos.zRot = 0;
					}
				}

				if (beetle->fallspeed >= 500)
				{
					beetle->on = false;
					NextScarab = 0;
				}
				else
				{
					beetle->pos.xRot = -64 * beetle->fallspeed;
				}
			}
		}
	}
}