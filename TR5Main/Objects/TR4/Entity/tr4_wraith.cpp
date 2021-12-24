#include "framework.h"
#include "tr4_wraith.h"
#include "Specific/level.h"
#include "Game/effects/effects.h"
#include "Game/room.h"
#include "Game/control/flipeffect.h"
#include "Objects/objectslist.h"
#include "Specific/trmath.h"
#include "Sound/sound.h"
#include "Game/Lara/lara.h"
#include "Objects/Generic/Traps/traps.h"
#include "Game/people.h"
#include "Game/effects/tomb4fx.h"
#include "Objects/TR4/Entity/tr4_wraith_info.h"
#include "Game/effects/lara_fx.h"
#include "Game/items.h"

using namespace TEN::Effects::Lara;

namespace TEN::Entities::TR4
{
	constexpr auto WRAITH_COUNT = 8;

	short WraithSpeed = 64;

	void InitialiseWraith(short itemNumber)
	{
		ITEM_INFO* item;

		item = &g_Level.Items[itemNumber];

		item->data = WRAITH_INFO();
		WRAITH_INFO* wraithData = item->data;

		item->itemFlags[0] = 0;
		item->itemFlags[6] = 0;
		item->speed = WraithSpeed;

		for (int i = 0; i < WRAITH_COUNT; i++)
		{
			wraithData->xPos = item->pos.xPos;
			wraithData->yPos = item->pos.yPos;
			wraithData->zPos = item->pos.zPos;
			wraithData->zRot = 0;
			wraithData->yPos = 0;
			wraithData->xPos = 0;
			wraithData->r = 0;
			wraithData->g = 0;
			wraithData->b = 0;

			wraithData++;
		}
	}

	void WraithControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		SoundEffect(SFX_TR4_WRAITH_WHISPERS, &item->pos, 0);

		// hitPoints stores the target of wraith
		ITEM_INFO* target;
		if (item->itemFlags[6])
			target = &g_Level.Items[item->itemFlags[6]];
		else
			target = LaraItem;

		int x, y, z, distance, dx, dy, dz, oldX, oldY, oldZ;

		if (target == LaraItem || target->objectNumber == ID_ANIMATING10)
		{
			x = target->pos.xPos - item->pos.xPos;
			y = target->pos.yPos;
			z = target->pos.zPos - item->pos.zPos;
			distance = SQUARE(x) + SQUARE(z);
			dy = abs((distance / 8192) - 512);
		}
		else
		{
			ROOM_INFO* room = &g_Level.Rooms[LaraItem->roomNumber];

			x = room->x + room->xSize * 1024 / 2 - item->pos.xPos;
			z = room->z + room->zSize * 1024 / 2 - item->pos.zPos;

			distance = SQUARE(x) + SQUARE(z);
			dy = abs((distance / MAX_VISIBILITY_DISTANCE) - 768);
			y = room->y + ((room->minfloor - room->maxceiling) / 2);
		}

		dy = y - item->pos.yPos - dy - 128;
		short angleH = phd_atan(z, x) - item->pos.yRot;

		short angleV = 0;
		if (abs(x) <= abs(z))
			angleV = phd_atan(abs(x) + (abs(z) / 2), dy);
		else
			angleV = phd_atan(abs(z) + (abs(x) / 2), dy);

		angleV -= item->pos.xRot;

		int speed = 8 * WraithSpeed / item->speed;

		if (abs(angleH) >= item->itemFlags[2] || angleH > 0 != item->itemFlags[2] > 0)
		{
			if (angleH >= 0)
			{
				if (item->itemFlags[2] <= 0)
				{
					item->itemFlags[2] = 1;
				}
				else
				{
					item->itemFlags[2] += speed;
					item->pos.yRot += item->itemFlags[2];
				}
			}
			else if (item->itemFlags[2] >= 0)
			{
				item->itemFlags[2] = -1;
			}
			else
			{
				item->itemFlags[2] -= speed;
				item->pos.yRot += item->itemFlags[2];
			}
		}
		else
		{
			item->pos.yRot += angleH;
		}

		if (abs(angleV) >= item->itemFlags[3] || angleV > 0 != item->itemFlags[3] > 0)
		{
			if (angleV >= 0)
			{
				if (item->itemFlags[3] <= 0)
				{
					item->itemFlags[3] = 1;
				}
				else
				{
					item->itemFlags[3] += speed;
					item->pos.xRot += item->itemFlags[3];
				}
			}
			else if (item->itemFlags[3] >= 0)
			{
				item->itemFlags[3] = -1;
			}
			else
			{
				item->itemFlags[3] -= speed;
				item->pos.xRot += item->itemFlags[3];
			}
		}
		else
		{
			item->pos.xRot += angleV;
		}

		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		int ceiling = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		bool hitWall = false;
		if (height < item->pos.yPos || ceiling > item->pos.yPos)
			hitWall = true;

		oldX = item->pos.xPos;
		oldY = item->pos.yPos;
		oldZ = item->pos.zPos;

		item->pos.xPos += item->speed * phd_sin(item->pos.yRot);
		item->pos.yPos += item->speed * phd_sin(item->pos.xRot);
		item->pos.zPos += item->speed * phd_cos(item->pos.yRot);

		auto outsideRoom = IsRoomOutside(item->pos.xPos, item->pos.yPos, item->pos.zPos);
		if (item->roomNumber != outsideRoom && outsideRoom != NO_ROOM)
		{
			ItemNewRoom(itemNumber, outsideRoom);

			auto r = &g_Level.Rooms[outsideRoom];
			short linkNum = NO_ITEM;
			for (linkNum = r->itemNumber; linkNum != NO_ITEM; linkNum = g_Level.Items[linkNum].nextItem)
			{
				ITEM_INFO* target = &g_Level.Items[linkNum];

				if (target->active)
				{
					if (item->objectNumber == ID_WRAITH1 && target->objectNumber == ID_WRAITH2 ||
						item->objectNumber == ID_WRAITH2 && target->objectNumber == ID_WRAITH1 ||
						item->objectNumber == ID_WRAITH3 && target->objectNumber == ID_ANIMATING10)
					{
						break;
					}
				}
			}

			if (linkNum != NO_ITEM)
			{
				item->itemFlags[6] = linkNum;
			}
		}

		if (item->objectNumber != ID_WRAITH3)
		{
			// WRAITH1 AND WRAITH2 can die on contact with water
			// WRAITH1 dies because it's fire and it dies on contact with water, WRAITH2 instead triggers a flipmap for making icy water
			if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
			{
				TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 2, -2, 1, item->roomNumber);
				item->itemFlags[1]--;
				if (item->itemFlags[1] < -1)
				{
					if (item->itemFlags[1] < 30)
					{
						if (item->objectNumber == ID_WRAITH2)
						{
							if (item->triggerFlags)
							{
								if (!FlipStats[item->triggerFlags])
								{
									DoFlipMap(item->triggerFlags);
									FlipStats[item->triggerFlags] = 1;
								}
							}
						}
						KillItem(itemNumber);
					}
				}
				else
				{
					item->itemFlags[1] = -1;
				}
			}
			else
			{
				item->itemFlags[1]--;
				if (item->itemFlags[1] < 0)
				{
					item->itemFlags[1] = 0;
				}
			}
		}

		if (distance >= 28900 || abs(item->pos.yPos - target->pos.yPos + 384) >= 256)
		{
			if (Wibble & 16)
			{
				if (item->speed < WraithSpeed)
				{
					item->speed++;
				}
				if (item->itemFlags[6])
				{
					if (item->itemFlags[7])
					{
						item->itemFlags[7]--;
					}
				}
			}
		}
		else
		{
			if (item->speed > 32)
			{
				item->speed -= 12;
			}
			if (target == LaraItem)
			{
				target->hitPoints -= distance / 1024;

				// WRAITH1 can burn Lara
				if (item->objectNumber == ID_WRAITH1)
				{
					item->itemFlags[1] += 400;
					if (item->itemFlags[1] > 8000)
					{
						LaraBurn(LaraItem);
					}
				}
			}
			else if (target->objectNumber == ID_ANIMATING10)
			{
				// ANIMATING10 is the sacred pedistal that can kill WRAITH
				item->itemFlags[7]++;
				if (item->itemFlags[7] > 10)
				{
					item->pos.xPos = target->pos.xPos;
					item->pos.yPos = target->pos.yPos - 384;
					item->pos.zPos = target->pos.zPos;
					WraithExplosionEffect(item, 96, 96, 96, -32);
					WraithExplosionEffect(item, 48, 48, 48, 48);
					target->triggerFlags--;
					target->hitPoints = 0;
					if (target->triggerFlags > 0)
					{
						target->frameNumber = g_Level.Anims[target->animNumber].frameBase;
					}
					KillItem(itemNumber);
				}
			}
			else
			{
				// Target is another WRAITH (fire vs ice), they kill both themselves
				target->itemFlags[7] = target->itemFlags[7] & 0x6A | 0xA;
				if (item->itemFlags[7])
				{
					TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 2, -2, 1, item->roomNumber);
					target->hitPoints = 0;
					KillItem(item->itemFlags[6]);
					KillItem(itemNumber);
				}
			}
		}

		// Check if WRAITH is going below floor or above ceiling and trigger sparks
		roomNumber = item->roomNumber;
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) < item->pos.yPos
			|| GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) > item->pos.yPos)
		{
			if (!hitWall)
			{
				WraithWallsEffect(oldX, oldY, oldZ, item->pos.yRot + -ANGLE(180), item->objectNumber);
			}
		}
		else if (hitWall)
		{
			WraithWallsEffect(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->pos.yRot, item->objectNumber);
		}

		// Update WRAITH nodes
		WRAITH_INFO* creature = (WRAITH_INFO*)item->data;
		int j = 0;
		for (int i = WRAITH_COUNT - 1; i > 0; i--)
		{
			creature[i - 1].xPos += (creature[i - 1].xRot / 16);
			creature[i - 1].yPos += (creature[i - 1].yRot / 16);
			creature[i - 1].zPos += (creature[i - 1].zRot / 16);

			creature[i - 1].xRot -= (creature[i - 1].xRot / 16);
			creature[i - 1].yRot -= (creature[i - 1].yRot / 16);
			creature[i - 1].zRot -= (creature[i - 1].zRot / 16);

			creature[i].xPos = creature[i - 1].xPos;
			creature[i].yPos = creature[i - 1].yPos;
			creature[i].zPos = creature[i - 1].zPos;

			creature[i].xRot = creature[i - 1].xRot;
			creature[i].yRot = creature[i - 1].yRot;
			creature[i].zRot = creature[i - 1].zRot;

			if (item->objectNumber == ID_WRAITH1)
			{
				creature[i].r = (GetRandomControl() & 0x3F) - 64;
				creature[i].g = 16 * (j + 1) + (GetRandomControl() & 0x3F);
				creature[i].b = GetRandomControl() & 0xF;
			}
			else if (item->objectNumber == ID_WRAITH2)
			{
				creature[i].r = GetRandomControl() & 0xF;
				creature[i].g = 16 * (j + 1) + (GetRandomControl() & 0x3F);
				creature[i].b = (GetRandomControl() & 0x3F) - 64;
			}
			else
			{
				creature[i].r = 8 * (j + 2) + (GetRandomControl() & 0x3F);
				creature[i].g = creature[i].r;
				creature[i].b = creature[i].r + (GetRandomControl() & 0xF);
			}

			j++;
		}

		creature[0].xPos = item->pos.xPos;
		creature[0].yPos = item->pos.yPos;
		creature[0].zPos = item->pos.zPos;

		creature[0].xRot = 4 * (item->pos.xPos - oldX);
		creature[0].yRot = 4 * (item->pos.yPos - oldY);
		creature[0].zRot = 4 * (item->pos.zPos - oldZ);

		// Standard WRAITH drawing code
		DrawWraith(
			item->pos.xPos,
			item->pos.yPos,
			item->pos.zPos,
			creature[0].xRot,
			creature[0].yRot,
			creature[0].zRot,
			item->objectNumber);

		DrawWraith(
			(oldX + item->pos.xPos) / 2,
			(oldY + item->pos.yPos) / 2,
			(oldZ + item->pos.zPos) / 2,
			creature[0].xRot,
			creature[0].yRot,
			creature[0].zRot,
			item->objectNumber);

		// Lighting for WRAITH
		byte r, g, b;
		if (item->objectNumber == ID_WRAITH3)
		{
			r = creature[5].r;
			g = creature[5].g;
			b = creature[5].b;
		}
		else
		{
			r = creature[1].r;
			g = creature[1].g;
			b = creature[1].b;
		}

		TriggerDynamicLight(
			creature[0].xPos,
			creature[0].yPos,
			creature[0].zPos,
			16,
			r, g, b);
	}

	void WraithExplosionEffect(ITEM_INFO* item, byte r, byte g, byte b, int speed)
	{
		short inner = speed >= 0 ? 32 : 640;
		short outer = speed >= 0 ? 160 : 512;

		item->pos.yPos -= 384;

		TriggerShockwave(&item->pos, inner, outer, speed, r, g, b, 24, 0, 0);
		TriggerShockwave(&item->pos, inner, outer, speed, r, g, b, 24, ANGLE(45), 0);
		TriggerShockwave(&item->pos, inner, outer, speed, r, g, b, 24, ANGLE(90), 0);
		TriggerShockwave(&item->pos, inner, outer, speed, r, g, b, 24, ANGLE(135), 0);

		item->pos.yPos += 384;
	}

	void DrawWraith(int x, int y, int z, short xVel, short yVel, short zVel, int objNumber)
	{
		unsigned char size, life;
		BYTE color;
		SPARKS* spark;
		spark = &Sparks[GetFreeSpark()];
		spark->on = 1;

		if (objNumber == ID_WRAITH1)
		{
			spark->sR = (GetRandomControl() & 0x1F) + -128;
			spark->sB = 24;
			spark->sG = (GetRandomControl() & 0x1F) + 48;
			spark->dR = (GetRandomControl() & 0x1F) + -128;
			spark->dB = 24;
			spark->dG = (GetRandomControl() & 0x1F) + 64;
		}
		else if (objNumber == ID_WRAITH2) {
			spark->sB = (GetRandomControl() & 0x1F) + -128;
			spark->sR = 24;
			spark->sG = (GetRandomControl() & 0x1F) + -128;
			spark->dB = (GetRandomControl() & 0x1F) + -128;
			spark->dR = 24;
			spark->dG = (GetRandomControl() & 0x1F) + 64;
		}
		else {
			color = (GetRandomControl() & 0x1F) + 64;
			spark->dG = color;
			spark->dR = color;
			spark->sB = color;
			spark->sG = color;
			spark->sR = color;
			spark->dB = spark->sB + (GetRandomControl() & 0x1F);
		}

		spark->colFadeSpeed = 4;
		spark->fadeToBlack = 7;
		spark->transType = TransTypeEnum::COLADD;
		life = (GetRandomControl() & 7) + 12;
		spark->life = life;
		spark->sLife = life;
		spark->x = (GetRandomControl() & 0x1F) + x - 16;
		spark->y = y;
		spark->friction = 85;
		spark->flags = SP_EXPDEF | SP_DEF | SP_SCALE;
		spark->z = (GetRandomControl() & 0x1F) + z - 16;
		spark->xVel = xVel;
		spark->yVel = yVel;
		spark->zVel = zVel;
		spark->gravity = 0;
		spark->maxYvel = 0;
		spark->scalar = 2;
		spark->dSize = 2;
		size = (GetRandomControl() & 0x1F) + 48;
		spark->sSize = size;
		spark->size = size;
	}

	void WraithWallsEffect(int x, int y, int z, short yrot, short objNumber)
	{
		byte sR, sG, sB, dR, dG, dB;
		short color;

		if (objNumber == ID_WRAITH1)
		{
			sR = (GetRandomControl() & 0x1F) + -128;
			sB = 24;
			sG = (GetRandomControl() & 0x1F) + 48;
			dR = (GetRandomControl() & 0x1F) + -128;
			dB = 24;
			dG = (GetRandomControl() & 0x1F) + 64;
		}
		else if (objNumber == ID_WRAITH2) {
			sB = (GetRandomControl() & 0x1F) + -128;
			sR = 24;
			sG = (GetRandomControl() & 0x1F) + -128;
			dB = (GetRandomControl() & 0x1F) + -128;
			dR = 24;
			dG = (GetRandomControl() & 0x1F) + 64;
		}
		else {
			color = (GetRandomControl() & 0x1F) + 64;
			dG = color;
			dR = color;
			sB = color;
			sG = color;
			sR = color;
			dB = sB + (GetRandomControl() & 0x1F);
		}

		for (int i = 0; i < 15; i++)
		{
			SPARKS* spark = &Sparks[GetFreeSpark()];
			spark->on = true;
			spark->sR = dR;
			spark->sG = dG;
			spark->sB = dB;
			spark->dR = dR;
			spark->dG = dG;
			spark->dB = dB;
			spark->colFadeSpeed = 4;
			spark->fadeToBlack = 7;
			spark->transType = TransTypeEnum::COLADD;
			short life = (GetRandomControl() & 7) + 32;
			spark->life = life;
			spark->sLife = life;
			spark->x = (GetRandomControl() & 0x1F) + x - 16;
			spark->y = (GetRandomControl() & 0x1F) + y - 16;
			spark->z = (GetRandomControl() & 0x1F) + z - 16;
			short rot = yrot + GetRandomControl() - ANGLE(90);
			short velocity = ((GetRandomControl() & 0x3FF) + 1024);
			spark->xVel = velocity * phd_sin(rot);
			spark->yVel = (GetRandomControl() & 0x7F) - 64;
			spark->zVel = velocity * phd_cos(rot);
			spark->friction = 4;
			spark->flags = SP_EXPDEF | SP_DEF | SP_SCALE;
			spark->maxYvel = 0;
			spark->scalar = 3;
			spark->gravity = (GetRandomControl() & 0x7F) - 64;
			short size = (GetRandomControl() & 0x1F) + 48;
			spark->sSize = size;
			spark->size = size;
			spark->dSize = size / 4;
		}
	}

	void KillWraith(ITEM_INFO* item)
	{
		ITEM_INFO* item2;
		item2 = nullptr;

		if (NextItemActive != NO_ITEM)
		{
			for (; NextItemActive != NO_ITEM;)
			{
				item2 = &g_Level.Items[NextItemActive];
				if (item2->objectNumber == ID_WRAITH3 && !item2->hitPoints)
				{
					break;
				}
				if (item2->nextActive == NO_ITEM)
				{
					FlipEffect = -1;
					return;
				}
			}
			item2->hitPoints = item - g_Level.Items.data();
		}
		FlipEffect = -1;
	}
}