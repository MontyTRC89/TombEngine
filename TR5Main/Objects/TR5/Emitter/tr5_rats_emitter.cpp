#include "framework.h"
#include "Objects/TR5/Emitter/tr5_rats_emitter.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
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

	if (item->TriggerFlags)
	{
		if (!item->ItemFlags[2] || !(GetRandomControl() & 0xF))
		{
			item->TriggerFlags--;

			if (item->ItemFlags[2] && GetRandomControl() & 1)
				item->ItemFlags[2]--;

			short ratNum = GetNextRat();
			if (ratNum != -1)
			{
				RAT_STRUCT* rat = &Rats[ratNum];

				rat->pos.Position.x = item->Pose.Position.x;
				rat->pos.Position.y = item->Pose.Position.y;
				rat->pos.Position.z = item->Pose.Position.z;
				rat->roomNumber = item->RoomNumber;

				if (item->ItemFlags[0])
				{
					rat->pos.Orientation.y = 2 * GetRandomControl();
					rat->fallspeed = -16 - (GetRandomControl() & 31);
				}
				else
				{
					rat->fallspeed = 0;
					rat->pos.Orientation.y = item->Pose.Orientation.y + (GetRandomControl() & 0x3FFF) - ANGLE(45);
				}

				rat->pos.Orientation.x = 0;
				rat->pos.Orientation.z = 0;
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

	char flags = item->TriggerFlags / 1000;

	item->Pose.Orientation.x = ANGLE(45);
	item->ItemFlags[1] = flags & 2;
	item->ItemFlags[2] = flags & 4;
	item->ItemFlags[0] = flags & 1;
	item->TriggerFlags = item->TriggerFlags % 1000;

	if (flags & 1)
	{
		ClearRats();
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
				int oldX = rat->pos.Position.x;
				int oldY = rat->pos.Position.y;
				int oldZ = rat->pos.Position.z;

				rat->pos.Position.x += rat->speed * phd_sin(rat->pos.Orientation.y);
				rat->pos.Position.y += rat->fallspeed;
				rat->pos.Position.z += rat->speed * phd_cos(rat->pos.Orientation.y);

				rat->fallspeed += GRAVITY;

				int dx = LaraItem->Pose.Position.x - rat->pos.Position.x;
				int dy = LaraItem->Pose.Position.y - rat->pos.Position.y;
				int dz = LaraItem->Pose.Position.z - rat->pos.Position.z;

				short angle;
				if (rat->flags >= 170)
					angle = rat->pos.Orientation.y - (short)phd_atan(dz, dx);
				else
					angle = (short)phd_atan(dz, dx) - rat->pos.Orientation.y;

				if (abs(dx) < 85 && abs(dy) < 85 && abs(dz) < 85)
				{
					LaraItem->HitPoints--;
					LaraItem->HitStatus = true;
				}

				// if life is even
				if (rat->flags & 1)
				{
					// if rat is very near
					if (abs(dz) + abs(dx) <= 1024)
					{
						if (rat->speed & 1)
							rat->pos.Orientation.y += 512;
						else
							rat->pos.Orientation.y -= 512;
						rat->speed = 48 - (abs(angle) / 1024);
					}
					else
					{
						if (rat->speed < (i & 31) + 24)
							rat->speed++;

						if (abs(angle) >= 2048)
						{
							if (angle >= 0)
								rat->pos.Orientation.y += 1024;
							else
								rat->pos.Orientation.y -= 1024;
						}
						else
						{
							rat->pos.Orientation.y += 8 * (Wibble - i);
						}
					}
				}

				short oldRoomNumber = rat->roomNumber;

				FLOOR_INFO* floor = GetFloor(rat->pos.Position.x, rat->pos.Position.y, rat->pos.Position.z,&rat->roomNumber);
				int height = GetFloorHeight(floor, rat->pos.Position.x, rat->pos.Position.y, rat->pos.Position.z);

				// if height is higher than 5 clicks 
				if (height < rat->pos.Position.y - 1280 ||
					height == NO_HEIGHT)
				{
					// if timer is higher than 170 time to disappear 
					if (rat->flags > 170)
					{
						rat->on = 0;
						NextRat = 0;
					}

					if (angle <= 0)
						rat->pos.Orientation.y -= ANGLE(90);
					else
						rat->pos.Orientation.y += ANGLE(90);

					// reset rat to old position and disable fall
					rat->pos.Position.x = oldX;
					rat->pos.Position.y = oldY;
					rat->pos.Position.z = oldZ;
					rat->fallspeed = 0;
				}
				else
				{
					// if height is lower than Y + 64
					if (height >= rat->pos.Position.y - 64)
					{
						// if rat is higher than floor
						if (height >= rat->pos.Position.y)
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
								rat->pos.Orientation.x = -128 * rat->fallspeed;
							}
						}
						else
						{
							rat->pos.Position.y = height;
							rat->fallspeed = 0;
							rat->flags |= 1;
						}
					}
					else
					{
						// if block is higher than rat position then run vertically
						rat->pos.Orientation.x = 14336;
						rat->pos.Position.x = oldX;
						rat->pos.Position.y = oldY - 24;
						rat->pos.Position.z = oldZ;
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
					rat->pos.Position.y = r->maxceiling + 50;

					if (g_Level.Rooms[oldRoomNumber].flags & ENV_FLAG_WATER)
					{
						if (!(GetRandomControl() & 0xF))
						{
							SetupRipple(rat->pos.Position.x, r->maxceiling, rat->pos.Position.z, (GetRandomControl() & 3) + 48, 2,Objects[ID_DEFAULT_SPRITES].meshIndex+SPR_RIPPLES);
						}
					}
					else
					{
						AddWaterSparks(rat->pos.Position.x, r->maxceiling, rat->pos.Position.z, 16);
						SetupRipple(rat->pos.Position.x, r->maxceiling, rat->pos.Position.z, (GetRandomControl() & 3) + 48, 2, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
						SoundEffect(1080,&rat->pos, 0);
					}
				}

				if (!i && !(GetRandomControl() & 4))
					SoundEffect(1079,&rat->pos, 0);
			}
		}
	}
}