#include "framework.h"
#include "Objects/TR5/Emitter/tr5_spider_emitter.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/control/flipeffect.h"
#include "Specific/setup.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Sound/sound.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"

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

	short flags = item->TriggerFlags / -24;

	item->Pose.Orientation.x = ANGLE(45);
	item->ItemFlags[1] = flags & 2;
	item->ItemFlags[2] = flags & 4;
	item->ItemFlags[0] = flags & 1;
	item->TriggerFlags = flags % 1000;

	if (flags & 1)
	{
		ClearSpiders();
		return;
	}

	if (item->Pose.Orientation.y > -28672 && item->Pose.Orientation.y < -4096)
	{
		item->Pose.Position.x += 512;
	}
	else if (item->Pose.Orientation.y > 4096 && item->Pose.Orientation.y < 28672)
	{
		item->Pose.Position.x -= 512;
	}

	if (item->Pose.Orientation.y > -8192 && item->Pose.Orientation.y < 8192)
	{
		item->Pose.Position.z -= 512;
	}
	else if (item->Pose.Orientation.y < -20480 || item->Pose.Orientation.y > 20480)
	{
		item->Pose.Position.z += 512;
	}

	ClearSpiders();
}

void SpidersEmitterControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->TriggerFlags)
	{
		if (!item->ItemFlags[2] || !(GetRandomControl() & 0xF))
		{
			item->TriggerFlags--;

			if (item->ItemFlags[2] && GetRandomControl() & 1)
				item->ItemFlags[2]--;

			short spiderNum = GetNextSpider();
			if (spiderNum != NO_ITEM)
			{
				SPIDER_STRUCT* spider = &Spiders[spiderNum];

				spider->pos.xPos = item->Pose.Position.x;
				spider->pos.yPos = item->Pose.Position.y;
				spider->pos.zPos = item->Pose.Position.z;
				spider->roomNumber = item->RoomNumber;

				if (item->ItemFlags[0])
				{
					spider->pos.yRot = 2 * GetRandomControl();
					spider->fallspeed = -16 - (GetRandomControl() & 0x1F);
				}
				else
				{
					spider->fallspeed = 0;
					spider->pos.yRot = item->Pose.Orientation.y + (GetRandomControl() & 0x3FFF) - ANGLE(45);
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

				int dx = LaraItem->Pose.Position.x - spider->pos.xPos;
				int dy = LaraItem->Pose.Position.y - spider->pos.yPos;
				int dz = LaraItem->Pose.Position.z - spider->pos.zPos;

				short angle = phd_atan(dz, dx) - spider->pos.yRot;

				if (abs(dx) < 85 && abs(dy) < 85 && abs(dz) < 85)
				{
					LaraItem->HitPoints -= 3;
					LaraItem->HitStatus = true;
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

				FLOOR_INFO* floor = GetFloor(spider->pos.xPos, spider->pos.yPos, spider->pos.zPos,&spider->roomNumber);
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
					SoundEffect(982,&spider->pos, 0);
			}
		}
	}
}