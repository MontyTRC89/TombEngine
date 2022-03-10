#include "framework.h"
#include "Objects/TR4/Object/tr4_clockwork_beetle.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Sound/sound.h"
#include "Game/collision/collide_room.h"

void ClockworkBeetleControl(short itemNumber)
{
	auto* beetle = &g_Level.Items[itemNumber];

	int flag = 0;

	if (LaraItem->AnimNumber == LA_MECHANICAL_BEETLE_USE)
	{
		short fb = g_Level.Anims[LA_MECHANICAL_BEETLE_USE].frameBase;

		if (LaraItem->FrameNumber < fb + 14)
		{
			beetle->Status = ITEM_INVISIBLE;
			return;
		}

		if (LaraItem->FrameNumber < fb + 104)
		{
			PHD_VECTOR pos = { 0, 0, -32 };
			GetLaraJointPosition(&pos, LM_RHAND);

			beetle->Position.xPos = pos.x;
			beetle->Position.yPos = pos.y;
			beetle->Position.zPos = pos.z;
			beetle->Status = ITEM_ACTIVE;
			beetle->Position.yRot = LaraItem->Position.yRot;
			beetle->Position.zRot = -0x31C4;
			return;
		}

		if (LaraItem->FrameNumber == fb + 104)
		{
			short roomNumber = beetle->RoomNumber;
			FLOOR_INFO* floor = GetFloor(beetle->Position.xPos, beetle->Position.yPos, beetle->Position.zPos, &roomNumber);
			int height = GetFloorHeight(floor, beetle->Position.xPos, beetle->Position.yPos, beetle->Position.zPos);

			if (abs(LaraItem->Position.yPos - height) > 64)
			{
				beetle->Position.xPos = LaraItem->Position.xPos;
				beetle->Position.yPos = LaraItem->Position.yPos;
				beetle->Position.zPos = LaraItem->Position.zPos;
			}

			return;
		}
	}

	SoundEffect(SFX_TR4_BEETLARA_WINDUP, &beetle->Position, 0);

	beetle->VerticalVelocity += 12;
	beetle->Position.yPos += beetle->VerticalVelocity;

	short roomNumber = beetle->RoomNumber;
	FLOOR_INFO* floor = GetFloor(beetle->Position.xPos, beetle->Position.yPos - 20, beetle->Position.zPos, &roomNumber);
	int height = GetFloorHeight(floor, beetle->Position.xPos, beetle->Position.yPos, beetle->Position.zPos);

	if (beetle->Position.yPos > height)
	{
		beetle->Position.yPos = height;

		if (beetle->VerticalVelocity <= 32)
			beetle->VerticalVelocity = 0;
		else
			beetle->VerticalVelocity = -beetle->VerticalVelocity >> 1;

		flag = 1;
	}

	TestTriggers(beetle, false);

	if (roomNumber != beetle->RoomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (beetle->ItemFlags[0])
	{
		beetle->Position.zRot = ANGLE(22.5f) * phd_sin(ANGLE(22.5f) * (GlobalCounter & 0xF));

		switch (beetle->ItemFlags[2])
		{
		case 0:
		{
			int x, z;

			x = (beetle->Position.xPos & -CLICK(2)) | 0x200;
			z = (beetle->Position.zPos & -CLICK(2)) | 0x200;
			x -= beetle->Position.xPos;
			z -= beetle->Position.zPos;

			if (x <= -8 || z <= -8 || x >= 8 || z >= 8)
			{
				int atan = phd_atan(z, x);
				short rot = atan - beetle->Position.yRot;

				if (abs(rot) > ANGLE(180.0f))
					rot = beetle->Position.yRot - atan;

				if (abs(rot) < ANGLE(1.4f))
				{
					beetle->Position.yRot = atan;
					beetle->ItemFlags[2] = 1;
				}
				else if (rot < 0)
					beetle->Position.yRot -= ANGLE(1.4f);
				else
					beetle->Position.yRot += ANGLE(1.4f);
			}
			else
			{
				beetle->Position.zPos &= -CLICK(2);
				beetle->Position.zPos &= -CLICK(2);
				beetle->ItemFlags[2] = 2;
			}

			break;
		}

		case 1:
		case 4:
		{
			int x, z;

			x = (beetle->Position.xPos & -CLICK(2)) | CLICK(2);
			z = (beetle->Position.zPos & -CLICK(2)) | CLICK(2);
			x -= beetle->Position.xPos;
			z -= beetle->Position.zPos;

			if (x <= -8 || z <= -8 || x >= 8 || z >= 8)
			{
				int atan = phd_atan(z, x);
				beetle->Position.yRot = atan;

				if (pow(x, 2) + pow(z, 2) >= 0x19000)
				{
					if (beetle->Velocity < 32)
						beetle->Velocity++;
				}
				else
				{
					if (beetle->Velocity <= 4)
					{
						if (beetle->Velocity < 4)
							beetle->Velocity++;
					}
					else
						beetle->Velocity = beetle->Velocity - (beetle->ItemFlags[2] == 4) - 1;
				}

				beetle->Position.xPos += beetle->Velocity * phd_sin(beetle->Position.yRot);
				beetle->Position.zPos += beetle->Velocity * phd_cos(beetle->Position.yRot);
			}
			else
			{
				beetle->Position.xPos = (beetle->Position.xPos & -512) | 0x200;
				beetle->Position.zPos = (beetle->Position.zPos & -512) | 0x200;

				if (beetle->ItemFlags[2] == 1)
					beetle->ItemFlags[2] = 2;
				else
				{
					Lara.Inventory.BeetleLife--;
					beetle->ItemFlags[2] = 5;
					short itemRoom = g_Level.Rooms[beetle->RoomNumber].itemNumber;

					if (itemRoom != NO_ITEM)
					{
						ITEM_INFO* item;
						short nextItem;

						while (true)
						{
							item = &g_Level.Items[itemRoom];
							nextItem = item->NextItem;

							if (item->ObjectNumber == ID_MAPPER)
							{
								int dx = beetle->Position.xPos - item->Position.xPos;
								int dy = beetle->Position.yPos - item->Position.yPos;
								int dz = beetle->Position.zPos - item->Position.zPos;

								if (dx > -SECTOR(1) && dx < SECTOR(1) &&
									dz > -SECTOR(1) && dz < SECTOR(1) &&
									dy > -SECTOR(1) && dy < SECTOR(1))
								{
									break;
								}
							}

							itemRoom = nextItem;

							if (itemRoom == NO_ITEM)
								return;
						}

						item->ItemFlags[0] = 1;
					}
				}
			}

			break;
		}

		case 2:
		{
			int rotation = beetle->ItemFlags[1] - beetle->Position.yRot;

			if (abs(rotation) > ANGLE(180.0f))
				rotation = beetle->Position.yRot - beetle->ItemFlags[1];

			if (abs(rotation) < ANGLE(1.4f))
			{
				beetle->ItemFlags[2] = 3;
				beetle->Position.yRot = beetle->ItemFlags[1];
			}
			else
			{
				if (rotation < 0)
					beetle->Position.yRot -= ANGLE(1.4f);
				else
					beetle->Position.yRot += ANGLE(1.4f);
			}

			break;
		}

		case 3:
		{
			if (beetle->Velocity < 32)
				beetle->Velocity++;

			beetle->Position.xPos += beetle->Velocity * phd_sin(beetle->Position.yRot);
			beetle->Position.zPos += beetle->Velocity * phd_cos(beetle->Position.yRot);

			if (!floor->Flags.MarkBeetle)
				beetle->ItemFlags[3] = 1;
			else
			{
				if (beetle->ItemFlags[3])
					beetle->ItemFlags[2] = 4;
			}

			break;
		}

		default:
			break;
		}
	}
	else
	{
		beetle->Position.zRot = ANGLE(45.0f) * phd_sin(ANGLE(45.0f) * (GlobalCounter & 0x7));

		if (beetle->ItemFlags[3])
			beetle->ItemFlags[3]--;

		if (Lara.Inventory.BeetleLife)
		{
			int val;

			if (beetle->ItemFlags[3] <= 75)
				val = beetle->ItemFlags[3];
			else
				val = 150 - beetle->ItemFlags[3];

			beetle->Position.yRot += 32 * val;
			val >>= 1;

			if (flag && beetle->ItemFlags[3] > 30 && val)
			{
				beetle->VerticalVelocity = -((val >> 1) + GetRandomControl() % val);
				return;
			}
		}
		else
		{
			beetle->Position.zRot *= 2;
			int val = (150 - beetle->ItemFlags[3]) >> 1;
			beetle->Position.yRot += val << 7;

			if (flag && val)
			{
				beetle->VerticalVelocity = -((val >> 1) + GetRandomControl() % val);
				return;
			}

			if (beetle->ItemFlags[3] < 30)
			{
				SoundEffect(102, &beetle->Position, 0);
				ExplodeItemNode(beetle, 0, 0, 128);
				KillItem(itemNumber);
			}
		}
	}
}

void UseClockworkBeetle(short flag)
{
	if (flag ||
		LaraItem->ActiveState == LS_IDLE &&
		LaraItem->AnimNumber == LA_STAND_IDLE &&
		!LaraItem->HitStatus &&
		Lara.Control.HandStatus == HandStatus::Free)
	{
		short itemNumber = CreateItem();

		if (itemNumber != NO_ITEM)
		{
			auto* item = &g_Level.Items[itemNumber];

			Lara.Inventory.BeetleComponents &= 0xFE;
			item->Shade = -15856;
			item->ObjectNumber = ID_CLOCKWORK_BEETLE;
			item->RoomNumber = LaraItem->RoomNumber;
			item->Position.xPos = LaraItem->Position.xPos;
			item->Position.yPos = LaraItem->Position.yPos;
			item->Position.zPos = LaraItem->Position.zPos;

			InitialiseItem(itemNumber);
			item->Position.xRot = 0;
			item->Position.yRot = LaraItem->Position.yRot;
			item->Position.zRot = 0;

			if (Lara.Inventory.BeetleLife)
				item->ItemFlags[0] = GetCollisionResult(item).Block->Flags.MarkBeetle;
			else
				item->ItemFlags[0] = 0;

			item->Velocity = 0;
			AddActiveItem(itemNumber);

			if (item->ItemFlags[0])
			{
				ITEM_INFO* item2;
				short itemRoom = g_Level.Rooms[item->RoomNumber].itemNumber;

				if (itemRoom != NO_ITEM)
				{
					while (true)
					{
						item2 = &g_Level.Items[itemRoom];
						short nextItem = item2->NextItem;

						if (item2->ObjectNumber == ID_MAPPER)
						{
							int dx = item->Position.xPos - item2->Position.xPos;
							int dy = item->Position.yPos - item2->Position.yPos;
							int dz = item->Position.zPos - item2->Position.zPos;

							if (dx > -SECTOR(1) && dx < SECTOR(1) &&
								dz > -SECTOR(1) && dz < SECTOR(1) &&
								dy > -SECTOR(1) && dy < SECTOR(1))
							{
								break;
							}
						}

						itemRoom = nextItem;

						if (itemRoom == NO_ITEM)
						{
							if (!item->ItemFlags[0])
								item->ItemFlags[3] = 150;

							return;
						}
					}

					item->ItemFlags[1] = item2->Position.yRot + 0x8000;

					if (item2->ItemFlags[0])
						item->ItemFlags[0] = 0;
					else
						item2->ItemFlags[0] = 1;
				}
			}

			if (!item->ItemFlags[0])
				item->ItemFlags[3] = 150;
		}
	}
}
