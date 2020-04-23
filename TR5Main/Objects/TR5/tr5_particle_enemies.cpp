#include "../oldobjects.h"
#include "../../Game/sphere.h"
#include "../../Game/items.h"
#include "../../Game/tomb4fx.h"
#include "../../Game/effect2.h"
#include "../../Game/Box.h"
#include "../../Game/people.h"
#include "../../Game/debris.h"
#include "../../Game/draw.h"
#include "../../Game/control.h"
#include "../../Game/effects.h"
#include "../../Specific/setup.h"
#include "..\..\Specific\level.h"
#include "../../Game/lara.h"

int NextBat;
BAT_STRUCT* Bats;

int NextSpider;
SPIDER_STRUCT* Spiders;

int NextRat;
RAT_STRUCT* Rats;

void InitialiseLittleBats(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (item->pos.yRot == 0)
	{
		item->pos.zPos += 512;
	}
	else if (item->pos.yRot == -ANGLE(180))
	{
		item->pos.zPos -= 512;
	}
	else if (item->pos.yRot == -ANGLE(90))
	{
		item->pos.xPos -= 512;
	}
	else if (item->pos.yRot == ANGLE(90))
	{
		item->pos.xPos += 512;
	}

	if (Objects[ID_BATS_EMITTER].loaded)
		ZeroMemory(Bats, NUM_BATS * sizeof(BAT_STRUCT));

	//LOWORD(item) = sub_402F27(ebx0, Bats, 0, 1920);
}

void ControlLittleBats(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (TriggerActive(item))
	{
		if (item->triggerFlags)
		{
			TriggerLittleBat(item);
			item->triggerFlags--;
		}
		else
		{
			KillItem(itemNumber);
		}
	}
}

short GetNextBat()
{
	short batNumber = NextBat;
	int index = 0;
	BAT_STRUCT* bat = &Bats[NextBat];

	while (bat->on)
	{
		if (batNumber == NUM_BATS - 1)
		{
			bat = (BAT_STRUCT*)Bats;
			batNumber = 0;
		}
		else
		{
			batNumber++;
			bat++;
		}

		index++;

		if (index >= NUM_BATS)
			return NO_ITEM;
	}

	NextBat = (batNumber + 1) & (NUM_BATS - 1);

	return batNumber;
}

void TriggerLittleBat(ITEM_INFO* item)
{
	short batNumber = GetNextBat();

	if (batNumber != NO_ITEM)
	{
		BAT_STRUCT* bat = &Bats[batNumber];

		bat->roomNumber = item->roomNumber;
		bat->pos.xPos = item->pos.xPos;
		bat->pos.yPos = item->pos.yPos;
		bat->pos.zPos = item->pos.zPos;
		bat->pos.yRot = (GetRandomControl() & 0x7FF) + item->pos.yRot + -ANGLE(180) - 1024;
		bat->on = 1;
		bat->flags = 0;
		bat->pos.xRot = (GetRandomControl() & 0x3FF) - 512;
		bat->speed = (GetRandomControl() & 0x1F) + 16;
		bat->laraTarget = GetRandomControl() & 0x1FF;
		bat->counter = 20 * ((GetRandomControl() & 7) + 15);
	}
}

short GetNextSpider()
{
	short spiderNum = NextSpider;
	int i = 0;
	SPIDER_STRUCT* spider = &Spiders[NextSpider];
	
	while (spider->on)
	{
		if (spiderNum == NUM_SPIDERS - 1)
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
			return -1;
	}

	NextSpider = (spiderNum + 1) & 0x3F;

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
	ITEM_INFO* item = &Items[itemNumber];

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
	else if(item->pos.yRot > 4096 && item->pos.yRot < 28672)
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

void ControlSpiders(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (item->triggerFlags)
	{
		if (!item->itemFlags[2] || !(GetRandomControl() & 0xF))
		{
			item->triggerFlags--;

			if (item->itemFlags[2] && GetRandomControl() & 1)
				item->itemFlags[2]--;
			
			short spiderNum = GetNextSpider();
			if (spiderNum != -1)
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

short GetNextRat()
{
	short ratNum = NextRat;
	int i = 0;
	RAT_STRUCT* rat = &Rats[NextRat];

	while (rat->on)
	{
		if (ratNum == NUM_RATS - 1)
		{
			rat = &Rats[0];
			ratNum = 0;
		}
		else
		{
			ratNum++;
			rat++;
		}

		i++;

		if (i >= NUM_RATS)
			return NO_ITEM;
	}

	NextRat = (ratNum + 1) & 0x1F;
	return ratNum;
}

void ControlLittleRats(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (item->triggerFlags)
	{
		if (!item->itemFlags[2] || !(GetRandomControl() & 0xF))
		{
			item->triggerFlags--;

			if (item->itemFlags[2] && GetRandomControl() & 1)
				item->itemFlags[2]--;

			short ratNum = GetNextRat();
			if (ratNum != -1)
			{
				RAT_STRUCT* rat = &Rats[ratNum];

				rat->pos.xPos = item->pos.xPos;
				rat->pos.yPos = item->pos.yPos;
				rat->pos.zPos = item->pos.zPos;
				rat->roomNumber = item->roomNumber;

				if (item->itemFlags[0])
				{
					rat->pos.yRot = 2 * GetRandomControl();
					rat->fallspeed = -16 - (GetRandomControl() & 31);
				}
				else
				{
					rat->fallspeed = 0;
					rat->pos.yRot = item->pos.yRot + (GetRandomControl() & 0x3FFF) - ANGLE(45);
				}

				rat->pos.xRot = 0;
				rat->pos.zRot = 0;
				rat->on = 1;
				rat->flags = GetRandomControl() & 30;
				rat->speed = (GetRandomControl() & 31) + 1;
			}
		}
	}
}

void ClearRats()
{
	if (Objects[ID_RATS_EMITTER].loaded)
	{
		ZeroMemory(Rats, NUM_RATS * sizeof(RAT_STRUCT));
		NextRat = 0;
		FlipEffect = -1;
	}
}

void InitialiseLittleRats(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	char flags = item->triggerFlags / 1000;

	item->pos.xRot = ANGLE(45);
	item->itemFlags[1] = flags & 2;
	item->itemFlags[2] = flags & 4;
	item->itemFlags[0] = flags & 1;
	item->triggerFlags = item->triggerFlags % 1000;

	if (flags & 1)
	{
		ClearRats();
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

	ClearRats();
}

void UpdateBats()
{
	if (!Objects[ID_BATS_EMITTER].loaded)
		return;

	short* bounds = GetBoundsAccurate(LaraItem);

	int x1 = LaraItem->pos.xPos + bounds[0] - (bounds[0] >> 2);
	int x2 = LaraItem->pos.xPos + bounds[1] - (bounds[1] >> 2);

	int y1 = LaraItem->pos.yPos + bounds[2] - (bounds[2] >> 2);
	int y2 = LaraItem->pos.yPos + bounds[3] - (bounds[3] >> 2);

	int z1 = LaraItem->pos.zPos + bounds[4] - (bounds[4] >> 2);
	int z2 = LaraItem->pos.zPos + bounds[5] - (bounds[5] >> 2);

	int minDistance = 0xFFFFFFF;
	int minIndex = -1;

	for (int i = 0; i < NUM_BATS; i++)
	{
		BAT_STRUCT* bat = &Bats[i];

		if (!bat->on)
			continue;

		if ((Lara.burn || LaraItem->hitPoints <= 0)
			&& bat->counter > 90
			&& !(GetRandomControl() & 7))
			bat->counter = 90;

		if (!(--bat->counter))
		{
			bat->on = 0;
			continue;
		}

		if (!(GetRandomControl() & 7))
		{
			bat->laraTarget = GetRandomControl() % 640 + 128;
			bat->xTarget = (GetRandomControl() & 0x7F) - 64;
			bat->zTarget = (GetRandomControl() & 0x7F) - 64;
		}

		short angles[2];
		phd_GetVectorAngles(
			LaraItem->pos.xPos + 8 * bat->xTarget - bat->pos.xPos,
			LaraItem->pos.yPos - bat->laraTarget - bat->pos.yPos,
			LaraItem->pos.zPos + 8 * bat->zTarget - bat->pos.zPos,
			angles);

		int distance = SQUARE(LaraItem->pos.zPos - bat->pos.zPos) +
					   SQUARE(LaraItem->pos.xPos - bat->pos.xPos);
		if (distance < minDistance)
		{
			minDistance = distance;
			minIndex = i;
		}

		distance = sqrt(distance) / 8;
		if (distance < 48)
			distance = 48;
		else if (distance > 128)
			distance = 128;

		if (bat->speed < distance)
			bat->speed++;
		else if (bat->speed > distance)
			bat->speed--;

		if (bat->counter > 90)
		{
			short speed = bat->speed << 7;

			short xAngle = abs(angles[1] - bat->pos.xRot) >> 3;
			short yAngle = abs(angles[0] - bat->pos.yRot) >> 3;

			if (xAngle < -speed)
				xAngle = -speed;
			else if (xAngle > speed)
				xAngle = speed;

			if (yAngle < -speed)
				yAngle = -speed;
			else if (yAngle > speed)
				yAngle = speed;

			bat->pos.yRot += yAngle;
			bat->pos.xRot += xAngle;
		}

		int sp = bat->speed * COS(bat->pos.xRot) >> W2V_SHIFT;

		bat->pos.xPos += sp * SIN(bat->pos.yRot) >> W2V_SHIFT;
		bat->pos.yPos += bat->speed * SIN(-bat->pos.xRot) >> W2V_SHIFT;
		bat->pos.zPos += sp * COS(bat->pos.yRot) >> W2V_SHIFT;

		if ((i % 2 == 0)
			&& bat->pos.xPos > x1
			&& bat->pos.xPos < x2
			&& bat->pos.yPos > y1
			&& bat->pos.yPos < y2
			&& bat->pos.zPos > z1
			&& bat->pos.zPos < z2)
		{
			TriggerBlood(bat->pos.xPos, bat->pos.yPos, bat->pos.zPos, 2 * GetRandomControl(), 2);
			if (LaraItem->hitPoints > 0)
				LaraItem->hitPoints -= 2;
		}
	}

	if (minIndex != -1)
	{
		BAT_STRUCT* bat = &Bats[minIndex];
		if (!(GetRandomControl() & 4))
			SoundEffect(SFX_BATS_1, &bat->pos, 0);
	}
}


void UpdateRats()
{
	if (Objects[ID_RATS_EMITTER].loaded)
	{
		for (int i = 0; i < NUM_RATS; i++)
		{
			RAT_STRUCT* rat = &Rats[i];

			if (rat->on)
			{
				int oldX = rat->pos.xPos;
				int oldY = rat->pos.yPos;
				int oldZ = rat->pos.zPos;

				rat->pos.xPos += rat->speed * SIN(rat->pos.yRot) >> W2V_SHIFT;
				rat->pos.yPos += rat->fallspeed;
				rat->pos.zPos += rat->speed * COS(rat->pos.yRot) >> W2V_SHIFT;

				rat->fallspeed += GRAVITY;

				int dx = LaraItem->pos.xPos - rat->pos.xPos;
				int dy = LaraItem->pos.yPos - rat->pos.yPos;
				int dz = LaraItem->pos.zPos - rat->pos.zPos;

				short angle;
				if (rat->flags >= 170)
					angle = rat->pos.yRot - (short)ATAN(dz, dx);
				else
					angle = (short)ATAN(dz, dx) - rat->pos.yRot;

				if (abs(dx) < 85 && abs(dy) < 85 && abs(dz) < 85)
				{
					LaraItem->hitPoints--;
					LaraItem->hitStatus = true;
				}

				// if life is even
				if (rat->flags & 1)
				{
					// if rat is very near
					if (abs(dz) + abs(dx) <= 1024)
					{
						if (rat->speed & 1)
							rat->pos.yRot += 512;
						else
							rat->pos.yRot -= 512;
						rat->speed = 48 - (abs(angle) >> 10);
					}
					else
					{
						if (rat->speed < (i & 31) + 24)
							rat->speed++;

						if (abs(angle) >= 2048)
						{
							if (angle >= 0)
								rat->pos.yRot += 1024;
							else
								rat->pos.yRot -= 1024;
						}
						else
						{
							rat->pos.yRot += 8 * (Wibble - i);
						}
					}
				}

				__int16 oldRoomNumber = rat->roomNumber;

				FLOOR_INFO* floor = GetFloor(rat->pos.xPos, rat->pos.yPos, rat->pos.zPos, &rat->roomNumber);
				int height = GetFloorHeight(floor, rat->pos.xPos, rat->pos.yPos, rat->pos.zPos);
				
				// if height is higher than 5 clicks 
				if (height < rat->pos.yPos - 1280 || 
					height == NO_HEIGHT)
				{
					// if timer is higher than 170 time to disappear 
					if (rat->flags > 170)
					{
						rat->on = 0;
						NextRat = 0;
					}

					if (angle <= 0)
						rat->pos.yRot -= ANGLE(90);
					else
						rat->pos.yRot += ANGLE(90);

					// reset rat to old position and disable fall
					rat->pos.xPos = oldX;
					rat->pos.yPos = oldY;
					rat->pos.zPos = oldZ;
					rat->fallspeed = 0;
				}
				else
				{
					// if height is lower than Y + 64
					if (height >= rat->pos.yPos - 64)
					{
						// if rat is higher than floor
						if (height >= rat->pos.yPos)
						{
							// if fallspeed is too much or life is ended then kill rat
							if (rat->fallspeed >= 500 || 
								rat->flags >= 200)
							{
								rat->on = 0;
								NextRat = 0;
							}
							else
							{
								rat->pos.xRot = -128 * rat->fallspeed;
							}
						}
						else
						{
							rat->pos.yPos = height;
							rat->fallspeed = 0;
							rat->flags |= 1;
						}
					}
					else
					{
						// if block is higher than rat position then run vertically
						rat->pos.xRot = 14336;
						rat->pos.xPos = oldX;
						rat->pos.yPos = oldY - 24;
						rat->pos.zPos = oldZ;
						rat->fallspeed = 0;
					}
				}

				if (!(Wibble & 60))
					rat->flags += 2;

				ROOM_INFO* r = &Rooms[rat->roomNumber];
				if (r->flags & ENV_FLAG_WATER)
				{
					rat->fallspeed = 0;
					rat->speed = 16;
					rat->pos.yPos = r->maxceiling + 50;

					if (Rooms[oldRoomNumber].flags & ENV_FLAG_WATER)
					{
						if (!(GetRandomControl() & 0xF))
						{
							SetupRipple(rat->pos.xPos, r->maxceiling, rat->pos.zPos, (GetRandomControl() & 3) + 48, 2);
						}
					}
					else
					{
						AddWaterSparks(rat->pos.xPos, r->maxceiling, rat->pos.zPos, 16);
						SetupRipple(rat->pos.xPos, r->maxceiling, rat->pos.zPos, (GetRandomControl() & 3) + 48, 2);
						SoundEffect(SFX_RATSPLASH, &rat->pos, 0);
					}
				}

				if (!i && !(GetRandomControl() & 4))
					SoundEffect(SFX_RATS_1, &rat->pos, 0);
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

				spider->pos.xPos += spider->speed * SIN(spider->pos.yRot) >> W2V_SHIFT;
				spider->pos.yPos += spider->fallspeed;
				spider->pos.zPos += spider->speed * COS(spider->pos.yRot) >> W2V_SHIFT;
				spider->fallspeed += GRAVITY;
				
				int dx = LaraItem->pos.xPos - spider->pos.xPos;
				int dy = LaraItem->pos.yPos - spider->pos.yPos;
				int dz = LaraItem->pos.zPos - spider->pos.zPos; 
				
				short angle = ATAN(dz, dx) - spider->pos.yRot;

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
						spider->speed = 48 - (abs(angle) >> 10);
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

				if (spider->pos.yPos < Rooms[spider->roomNumber].maxceiling + 50)
				{
					spider->fallspeed = 1;
					spider->pos.yRot += -32768;
					spider->pos.yPos = Rooms[spider->roomNumber].maxceiling + 50;
				}

				if (!i && !(GetRandomControl() & 4))
					SoundEffect(SFX_BEETLES, &spider->pos, 0);
			}
		}
	}
}