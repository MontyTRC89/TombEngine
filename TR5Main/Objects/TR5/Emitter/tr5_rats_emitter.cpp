#include "framework.h"
#include "Objects/TR5/Emitter/tr5_rats_emitter.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/control/flipeffect.h"
#include "Specific/setup.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Sound/sound.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"

int NextRat;
RAT_STRUCT Rats[NUM_RATS];

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

void LittleRatsControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

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
	ITEM_INFO* item = &g_Level.Items[itemNumber];

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

				rat->pos.xPos += rat->speed * phd_sin(rat->pos.yRot);
				rat->pos.yPos += rat->fallspeed;
				rat->pos.zPos += rat->speed * phd_cos(rat->pos.yRot);

				rat->fallspeed += GRAVITY;

				int dx = LaraItem->pos.xPos - rat->pos.xPos;
				int dy = LaraItem->pos.yPos - rat->pos.yPos;
				int dz = LaraItem->pos.zPos - rat->pos.zPos;

				short angle;
				if (rat->flags >= 170)
					angle = rat->pos.yRot - (short)phd_atan(dz, dx);
				else
					angle = (short)phd_atan(dz, dx) - rat->pos.yRot;

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
						rat->speed = 48 - (abs(angle) / 1024);
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

				short oldRoomNumber = rat->roomNumber;

				FLOOR_INFO* floor = GetFloor(rat->pos.xPos, rat->pos.yPos, rat->pos.zPos,&rat->roomNumber);
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

				ROOM_INFO* r = &g_Level.Rooms[rat->roomNumber];
				if (r->flags & ENV_FLAG_WATER)
				{
					rat->fallspeed = 0;
					rat->speed = 16;
					rat->pos.yPos = r->maxceiling + 50;

					if (g_Level.Rooms[oldRoomNumber].flags & ENV_FLAG_WATER)
					{
						if (!(GetRandomControl() & 0xF))
						{
							SetupRipple(rat->pos.xPos, r->maxceiling, rat->pos.zPos, (GetRandomControl() & 3) + 48, 2,Objects[ID_DEFAULT_SPRITES].meshIndex+SPR_RIPPLES);
						}
					}
					else
					{
						AddWaterSparks(rat->pos.xPos, r->maxceiling, rat->pos.zPos, 16);
						SetupRipple(rat->pos.xPos, r->maxceiling, rat->pos.zPos, (GetRandomControl() & 3) + 48, 2, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
						SoundEffect(1080,&rat->pos, 0);
					}
				}

				if (!i && !(GetRandomControl() & 4))
					SoundEffect(1079,&rat->pos, 0);
			}
		}
	}
}