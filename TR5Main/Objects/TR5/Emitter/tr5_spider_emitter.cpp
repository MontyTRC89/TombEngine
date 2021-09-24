#include "framework.h"
#include "tr5_spider_emitter.h"
#include "level.h"
#include "control/control.h"
#include "flipeffect.h"
#include "setup.h"
#include "effects\effects.h"
#include "effects\tomb4fx.h"
#include "Sound\sound.h"
#include "lara.h"
#include "item.h"

int NextSpider;
SPIDER_STRUCT Spiders[NUM_SPIDERS];

short GetNextSpider()
{
	short spiderNum = NextSpider;
	int i = 0;
	SPIDER_STRUCT* spider = &Spiders[NextSpider];

	while (spider->on)
	{
		if (spiderNum == (NUM_SPIDERS - 1))
		{
			spider = &Spiders[0];
			spiderNum = 0;
		}
		else
		{
			++spiderNum;
			++spider;
		}

		if (++i >= NUM_SPIDERS)
			return NO_ITEM;
	}

	NextSpider = (spiderNum + 1) & (NUM_SPIDERS - 1);
	return spiderNum;
}

void ClearSpiders()
{
	if (Objects[ID_SPIDERS_EMITTER].loaded)
	{
		ZeroMemory(Spiders, NUM_SPIDERS * sizeof(SPIDER_STRUCT));
		NextSpider = 0;
		FlipEffect = -1;
	}
}

void ClearSpidersPatch(ITEM_INFO* item)
{
	ClearSpiders();
}

void InitialiseSpiders(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	short flags = item->triggerFlags / -24;

	item->pos.xRot = ANGLE(45);
	item->itemFlags[1] = flags & 2;
	item->itemFlags[2] = flags & 4;
	item->itemFlags[0] = flags & 1;
	item->triggerFlags = flags % 1000;

	if (flags & 1)
	{
		ClearSpiders();
		return;
	}

	if (item->pos.yRot > -28672 && item->pos.yRot < -4096)
	{
		item->pos.xPos += 512;
	}
	else if (item->pos.yRot > 4096 && item->pos.yRot < 28672)
	{
		item->pos.xPos -= 512;
	}

	if (item->pos.yRot > -8192 && item->pos.yRot < 8192)
	{
		item->pos.zPos -= 512;
	}
	else if (item->pos.yRot < -20480 || item->pos.yRot > 20480)
	{
		item->pos.zPos += 512;
	}

	ClearSpiders();
}

void SpidersEmitterControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->triggerFlags)
	{
		if (!item->itemFlags[2] || !(GetRandomControl() & 0xF))
		{
			item->triggerFlags--;

			if (item->itemFlags[2] && GetRandomControl() & 1)
				item->itemFlags[2]--;

			short spiderNum = GetNextSpider();
			if (spiderNum != NO_ITEM)
			{
				SPIDER_STRUCT* spider = &Spiders[spiderNum];

				spider->pos.xPos = item->pos.xPos;
				spider->pos.yPos = item->pos.yPos;
				spider->pos.zPos = item->pos.zPos;
				spider->roomNumber = item->roomNumber;

				if (item->itemFlags[0])
				{
					spider->pos.yRot = 2 * GetRandomControl();
					spider->fallspeed = -16 - (GetRandomControl() & 0x1F);
				}
				else
				{
					spider->fallspeed = 0;
					spider->pos.yRot = item->pos.yRot + (GetRandomControl() & 0x3FFF) - ANGLE(45);
				}

				spider->pos.xRot = 0;
				spider->pos.zRot = 0;
				spider->on = true;
				spider->flags = 0;
				spider->speed = (GetRandomControl() & 0x1F) + 1;
			}
		}
	}
}

void UpdateSpiders()
{
	if (Objects[ID_SPIDERS_EMITTER].loaded)
	{
		for (int i = 0; i < NUM_SPIDERS; i++)
		{
			SPIDER_STRUCT* spider = &Spiders[i];
			if (spider->on)
			{
				int x = spider->pos.xPos;
				int y = spider->pos.yPos;
				int z = spider->pos.zPos;

				spider->pos.xPos += spider->speed * phd_sin(spider->pos.yRot);
				spider->pos.yPos += spider->fallspeed;
				spider->pos.zPos += spider->speed * phd_cos(spider->pos.yRot);
				spider->fallspeed += GRAVITY;

				int dx = LaraItem->pos.xPos - spider->pos.xPos;
				int dy = LaraItem->pos.yPos - spider->pos.yPos;
				int dz = LaraItem->pos.zPos - spider->pos.zPos;

				short angle = phd_atan(dz, dx) - spider->pos.yRot;

				if (abs(dx) < 85 && abs(dy) < 85 && abs(dz) < 85)
				{
					LaraItem->hitPoints -= 3;
					LaraItem->hitStatus = true;
					TriggerBlood(spider->pos.xPos, spider->pos.yPos, spider->pos.zPos, spider->pos.yRot, 1);
				}

				if (spider->flags)
				{
					if (abs(dx) + abs(dz) <= 768)
					{
						if (spider->speed & 1)
							spider->pos.yRot += 512;
						else
							spider->pos.yRot -= 512;
						spider->speed = 48 - (abs(angle) / 1024);
					}
					else
					{
						if (spider->speed < (i & 0x1F) + 24)
							spider->speed++;

						if (abs(angle) >= 2048)
						{
							if (angle >= 0)
								spider->pos.yRot += 1024;
							else
								spider->pos.yRot -= 1024;
						}
						else
						{
							spider->pos.yRot += 8 * (Wibble - i);
						}
					}
				}

				FLOOR_INFO* floor = GetFloor(spider->pos.xPos, spider->pos.yPos, spider->pos.zPos, &spider->roomNumber);
				int height = GetFloorHeight(floor, spider->pos.xPos, spider->pos.yPos, spider->pos.zPos);

				if (height >= spider->pos.yPos - 1280 || height == -32512)
				{
					if (height >= spider->pos.yPos - 64)
					{
						if (spider->pos.yPos <= height)
						{
							if (spider->fallspeed >= 500)
							{
								spider->on = false;
								NextSpider = 0;
							}
							else
							{
								spider->pos.xRot = -128 * spider->fallspeed;
							}
						}
						else
						{
							spider->pos.yPos = height;
							spider->fallspeed = 0;
							spider->flags = 1;
						}
					}
					else
					{
						spider->pos.xRot = 14336;
						spider->pos.xPos = x;
						spider->pos.yPos = y - 8;
						spider->pos.zPos = z;
						spider->fallspeed = 0;
						if (!(GetRandomControl() & 0x1F))
							spider->pos.yRot += -ANGLE(180);
					}
				}
				else
				{
					if (angle <= 0)
						spider->pos.yRot -= ANGLE(90);
					else
						spider->pos.yRot += ANGLE(90);
					spider->pos.xPos = x;
					spider->pos.yPos = y;
					spider->pos.zPos = z;
					spider->fallspeed = 0;
				}

				if (spider->pos.yPos < g_Level.Rooms[spider->roomNumber].maxceiling + 50)
				{
					spider->fallspeed = 1;
					spider->pos.yRot += -32768;
					spider->pos.yPos = g_Level.Rooms[spider->roomNumber].maxceiling + 50;
				}

				if (!i && !(GetRandomControl() & 4))
					SoundEffect(SFX_TR5_BEETLES, &spider->pos, 0);
			}
		}
	}
}