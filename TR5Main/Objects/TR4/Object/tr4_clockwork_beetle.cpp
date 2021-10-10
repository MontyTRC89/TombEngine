#include "framework.h"
#include "tr4_clockwork_beetle.h"
#include "items.h"
#include "level.h"
#include "lara.h"
#include "animation.h"
#include "Sound/sound.h"
#include "collide.h"

void ClockworkBeetleControl(short item_number)
{
	ITEM_INFO* beetle = &g_Level.Items[item_number];
	int flag;

	flag = 0;

	if (LaraItem->animNumber == LA_MECHANICAL_BEETLE_USE)
	{
		short fb = g_Level.Anims[LA_MECHANICAL_BEETLE_USE].frameBase;

		if (LaraItem->frameNumber < fb + 14)
		{
			beetle->status = ITEM_INVISIBLE;
			return;
		}

		if (LaraItem->frameNumber < fb + 104)
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = -32;
			GetLaraJointPosition(&pos, LM_RHAND);

			beetle->pos.xPos = pos.x;
			beetle->pos.yPos = pos.y;
			beetle->pos.zPos = pos.z;
			beetle->status = ITEM_ACTIVE;
			beetle->pos.yRot = LaraItem->pos.yRot;
			beetle->pos.zRot = -0x31C4;
			return;
		}

		if (LaraItem->frameNumber == fb + 104)
		{
			short roomNum = beetle->roomNumber;
			FLOOR_INFO* floor = GetFloor(beetle->pos.xPos, beetle->pos.yPos, beetle->pos.zPos, &roomNum);
			int height = GetFloorHeight(floor, beetle->pos.xPos, beetle->pos.yPos, beetle->pos.zPos);

			if (abs(LaraItem->pos.yPos - height) > 64)
			{
				beetle->pos.xPos = LaraItem->pos.xPos;
				beetle->pos.yPos = LaraItem->pos.yPos;
				beetle->pos.zPos = LaraItem->pos.zPos;
			}

			return;
		}
	}

	SoundEffect(SFX_TR4_BEETLARA_WINDUP, &beetle->pos, 0);

	beetle->fallspeed += 12;
	beetle->pos.yPos += beetle->fallspeed;

	short roomNum = beetle->roomNumber;
	FLOOR_INFO* floor = GetFloor(beetle->pos.xPos, beetle->pos.yPos - 20, beetle->pos.zPos, &roomNum);
	int height = GetFloorHeight(floor, beetle->pos.xPos, beetle->pos.yPos, beetle->pos.zPos);

	if (beetle->pos.yPos > height)
	{
		beetle->pos.yPos = height;

		if (beetle->fallspeed <= 32)
			beetle->fallspeed = 0;
		else
			beetle->fallspeed = -beetle->fallspeed >> 1;

		flag = 1;
	}

	TestTriggers(beetle, false);

	if (roomNum != beetle->roomNumber)
		ItemNewRoom(item_number, roomNum);

	if (beetle->itemFlags[0])
	{
		beetle->pos.zRot = 4096 * phd_sin(4096 * (GlobalCounter & 0xF));

		switch (beetle->itemFlags[2])
		{
		case 0:
		{
			int x, z;

			x = (beetle->pos.xPos & -512) | 0x200;
			z = (beetle->pos.zPos & -512) | 0x200;
			x -= beetle->pos.xPos;
			z -= beetle->pos.zPos;

			if (x <= -8 || z <= -8 || x >= 8 || z >= 8)
			{
				int atan = phd_atan(z, x);
				short rot = atan - beetle->pos.yRot;

				if (abs(rot) > 0x8000)
					rot = beetle->pos.yRot - atan;

				if (abs(rot) < 256)
				{
					beetle->pos.yRot = atan;
					beetle->itemFlags[2] = 1;
				}
				else if (rot < 0)
					beetle->pos.yRot -= 256;
				else
					beetle->pos.yRot += 256;
			}
			else
			{
				beetle->pos.zPos &= -512;
				beetle->pos.zPos &= -512;
				beetle->itemFlags[2] = 2;
			}

			break;
		}

		case 1:
		case 4:
		{
			int x, z;

			x = (beetle->pos.xPos & -512) | 0x200;
			z = (beetle->pos.zPos & -512) | 0x200;
			x -= beetle->pos.xPos;
			z -= beetle->pos.zPos;

			if (x <= -8 || z <= -8 || x >= 8 || z >= 8)
			{
				int atan = phd_atan(z, x);
				beetle->pos.yRot = atan;

				if (SQUARE(x) + SQUARE(z) >= 0x19000)
				{
					if (beetle->speed < 32)
						beetle->speed++;
				}
				else
				{
					if (beetle->speed <= 4)
					{
						if (beetle->speed < 4)
							beetle->speed++;
					}
					else
						beetle->speed = beetle->speed - (beetle->itemFlags[2] == 4) - 1;
				}

				beetle->pos.xPos += beetle->speed * phd_sin(beetle->pos.yRot);
				beetle->pos.zPos += beetle->speed * phd_cos(beetle->pos.yRot);
			}
			else
			{
				beetle->pos.xPos = (beetle->pos.xPos & -512) | 0x200;
				beetle->pos.zPos = (beetle->pos.zPos & -512) | 0x200;

				if (beetle->itemFlags[2] == 1)
					beetle->itemFlags[2] = 2;
				else
				{
					Lara.BeetleLife--;
					beetle->itemFlags[2] = 5;
					short room_item = g_Level.Rooms[beetle->roomNumber].itemNumber;

					if (room_item != NO_ITEM)
					{
						ITEM_INFO* item;
						int dx, dy, dz;
						short nex;

						while (1)
						{
							item = &g_Level.Items[room_item];
							nex = item->nextItem;

							if (item->objectNumber == ID_MAPPER)
							{
								dx = beetle->pos.xPos - item->pos.xPos;
								dy = beetle->pos.yPos - item->pos.yPos;
								dz = beetle->pos.zPos - item->pos.zPos;
								if (dx > -1024 && dx < 1024 && dz > -1024 && dz < 1024 && dy > -1024 && dy < 1024)
									break;
							}

							room_item = nex;

							if (room_item == NO_ITEM)
								return;
						}

						item->itemFlags[0] = 1;
					}
				}
			}

			break;
		}

		case 2:
		{
			int rot = beetle->itemFlags[1] - beetle->pos.yRot;

			if (abs(rot) > 0x8000)
				rot = beetle->pos.yRot - beetle->itemFlags[1];

			if (abs(rot) < 256)
			{
				beetle->itemFlags[2] = 3;
				beetle->pos.yRot = beetle->itemFlags[1];
			}
			else
			{
				if (rot < 0)
					beetle->pos.yRot -= 256;
				else
					beetle->pos.yRot += 256;
			}

			break;
		}

		case 3:
		{
			if (beetle->speed < 32)
				beetle->speed++;

			beetle->pos.xPos += beetle->speed * phd_sin(beetle->pos.yRot);
			beetle->pos.zPos += beetle->speed * phd_cos(beetle->pos.yRot);

			if (!floor->Flags.MarkBeetle)
				beetle->itemFlags[3] = 1;
			else
			{
				if (beetle->itemFlags[3])
					beetle->itemFlags[2] = 4;
			}

			break;
		}

		default:
			break;
		}
	}
	else
	{
		beetle->pos.zRot = 8192 * phd_sin(8192 * (GlobalCounter & 0x7));

		if (beetle->itemFlags[3])
			beetle->itemFlags[3]--;

		if (Lara.BeetleLife)
		{
			int val;

			if (beetle->itemFlags[3] <= 75)
				val = beetle->itemFlags[3];
			else
				val = 150 - beetle->itemFlags[3];

			beetle->pos.yRot += 32 * val;
			val >>= 1;

			if (flag && beetle->itemFlags[3] > 30 && val)
			{
				beetle->fallspeed = -((val >> 1) + GetRandomControl() % val);
				return;
			}
		}
		else
		{
			beetle->pos.zRot *= 2;
			int val = (150 - beetle->itemFlags[3]) >> 1;
			beetle->pos.yRot += val << 7;

			if (flag && val)
			{
				beetle->fallspeed = -((val >> 1) + GetRandomControl() % val);
				return;
			}

			if (beetle->itemFlags[3] < 30)
			{
				SoundEffect(102, &beetle->pos, 0);
				ExplodeItemNode(beetle, 0, 0, 128);
				KillItem(item_number);
			}
		}
	}
}

void UseClockworkBeetle(short flag)
{
	ITEM_INFO* item;
	short itemNum;

	if (flag
		|| LaraItem->currentAnimState == LS_STOP
		&& LaraItem->animNumber == LA_STAND_IDLE
		&& !LaraItem->hitStatus
		&& Lara.gunStatus == LG_NO_ARMS)
	{
		itemNum = CreateItem();

		if (itemNum != NO_ITEM)
		{
			item = &g_Level.Items[itemNum];
			Lara.hasBeetleThings &= 0xFE;
			item->shade = -15856;
			item->objectNumber = ID_CLOCKWORK_BEETLE;
			item->roomNumber = LaraItem->roomNumber;
			item->pos.xPos = LaraItem->pos.xPos;
			item->pos.yPos = LaraItem->pos.yPos;
			item->pos.zPos = LaraItem->pos.zPos;
			InitialiseItem(itemNum);
			item->pos.zRot = 0;
			item->pos.xRot = 0;
			item->pos.yRot = LaraItem->pos.yRot;

			if (Lara.BeetleLife)
				item->itemFlags[0] = GetCollisionResult(item).Block->Flags.MarkBeetle;
			else
				item->itemFlags[0] = 0;

			item->speed = 0;
			AddActiveItem(itemNum);

			if (item->itemFlags[0])
			{
				ITEM_INFO* item2;
				short roomItem, nex;
				int dx, dy, dz;

				roomItem = g_Level.Rooms[item->roomNumber].itemNumber;

				if (roomItem != NO_ITEM)
				{
					while (1)
					{
						item2 = &g_Level.Items[roomItem];
						nex = item2->nextItem;

						if (item2->objectNumber == ID_MAPPER)
						{
							dx = item->pos.xPos - item2->pos.xPos;
							dy = item->pos.yPos - item2->pos.yPos;
							dz = item->pos.zPos - item2->pos.zPos;
							if (dx > -1024 && dx < 1024 && dz > -1024 && dz < 1024 && dy > -1024 && dy < 1024)
								break;
						}
						roomItem = nex;

						if (roomItem == NO_ITEM)
						{
							if (!item->itemFlags[0])
								item->itemFlags[3] = 150;

							return;
						}
					}

					item->itemFlags[1] = item2->pos.yRot + 0x8000;

					if (item2->itemFlags[0])
						item->itemFlags[0] = 0;
					else
						item2->itemFlags[0] = 1;
				}
			}

			if (!item->itemFlags[0])
				item->itemFlags[3] = 150;
		}
	}
}
