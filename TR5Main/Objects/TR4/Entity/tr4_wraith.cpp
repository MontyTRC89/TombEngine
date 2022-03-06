#include "framework.h"
#include "tr4_wraith.h"
#include "Specific/level.h"
#include "Game/effects/effects.h"
#include "Game/room.h"
#include "Game/control/flipeffect.h"
#include "Objects/objectslist.h"
#include "Specific/trmath.h"
#include "Sound/sound.h"
#include "Game/collision/collide_room.h"
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

	auto WraithVelocity = 64;

	void InitialiseWraith(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->Data = WraithInfo();
		auto* wraith = (WraithInfo*)item->Data;

		item->VerticalVelocity = WraithVelocity;
		item->ItemFlags[0] = 0;
		item->ItemFlags[6] = 0;

		for (int i = 0; i < WRAITH_COUNT; i++)
		{
			wraith->Position.xPos = item->Position.xPos;
			wraith->Position.yPos = item->Position.yPos;
			wraith->Position.zPos = item->Position.zPos;
			wraith->Position.zRot = 0;
			wraith->Position.yPos = 0;
			wraith->Position.xPos = 0;
			wraith->r = 0;
			wraith->g = 0;
			wraith->b = 0;

			wraith++;
		}
	}

	void WraithControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		SoundEffect(SFX_TR4_WRAITH_WHISPERS, &item->Position, 0);

		// HitPoints stores the target of wraith
		auto* target = item->ItemFlags[6] ? &g_Level.Items[item->ItemFlags[6]] : LaraItem;

		int x, y, z, distance, dx, dy, dz, oldX, oldY, oldZ;

		if (target == LaraItem || target->ObjectNumber == ID_ANIMATING10)
		{
			x = target->Position.xPos - item->Position.xPos;
			y = target->Position.yPos;
			z = target->Position.zPos - item->Position.zPos;
			distance = pow(x, 2) + pow(z, 2);
			dy = abs((distance / SECTOR(8)) - SECTOR(0.5f));
		}
		else
		{
			auto* room = &g_Level.Rooms[LaraItem->RoomNumber];

			x = room->x + room->xSize * SECTOR(1) / 2 - item->Position.xPos;
			z = room->z + room->zSize * SECTOR(1) / 2 - item->Position.zPos;

			distance = pow(x, 2) + pow(z, 2);
			dy = abs((distance / MAX_VISIBILITY_DISTANCE) - CLICK(1));
			y = room->y + ((room->minfloor - room->maxceiling) / 2);
		}

		dy = y - item->Position.yPos - dy - CLICK(0.5f);
		short angleH = phd_atan(z, x) - item->Position.yRot;

		short angleV = 0;
		if (abs(x) <= abs(z))
			angleV = phd_atan(abs(x) + (abs(z) / 2), dy);
		else
			angleV = phd_atan(abs(z) + (abs(x) / 2), dy);

		angleV -= item->Position.xRot;

		int velocity = 8 * (WraithVelocity / item->VerticalVelocity);

		if (abs(angleH) >= item->ItemFlags[2] || angleH > 0 != item->ItemFlags[2] > 0)
		{
			if (angleH >= 0)
			{
				if (item->ItemFlags[2] <= 0)
					item->ItemFlags[2] = 1;
				else
				{
					item->ItemFlags[2] += velocity;
					item->Position.yRot += item->ItemFlags[2];
				}
			}
			else if (item->ItemFlags[2] >= 0)
				item->ItemFlags[2] = -1;
			else
			{
				item->ItemFlags[2] -= velocity;
				item->Position.yRot += item->ItemFlags[2];
			}
		}
		else
			item->Position.yRot += angleH;

		if (abs(angleV) >= item->ItemFlags[3] || angleV > 0 != item->ItemFlags[3] > 0)
		{
			if (angleV >= 0)
			{
				if (item->ItemFlags[3] <= 0)
					item->ItemFlags[3] = 1;
				else
				{
					item->ItemFlags[3] += velocity;
					item->Position.xRot += item->ItemFlags[3];
				}
			}
			else if (item->ItemFlags[3] >= 0)
				item->ItemFlags[3] = -1;
			else
			{
				item->ItemFlags[3] -= velocity;
				item->Position.xRot += item->ItemFlags[3];
			}
		}
		else
			item->Position.xRot += angleV;

		auto probe = GetCollisionResult(item);

		bool hitWall = false;
		if (probe.Position.Floor < item->Position.yPos || probe.Position.Ceiling > item->Position.yPos)
			hitWall = true;

		oldX = item->Position.xPos;
		oldY = item->Position.yPos;
		oldZ = item->Position.zPos;

		item->Position.xPos += item->VerticalVelocity * phd_sin(item->Position.yRot);
		item->Position.yPos += item->VerticalVelocity * phd_sin(item->Position.xRot);
		item->Position.zPos += item->VerticalVelocity * phd_cos(item->Position.yRot);

		auto outsideRoom = IsRoomOutside(item->Position.xPos, item->Position.yPos, item->Position.zPos);
		if (item->RoomNumber != outsideRoom && outsideRoom != NO_ROOM)
		{
			ItemNewRoom(itemNumber, outsideRoom);

			auto* room = &g_Level.Rooms[outsideRoom];
			short linkNumber = NO_ITEM;
			for (linkNumber = room->itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
			{
				auto* target = &g_Level.Items[linkNumber];

				if (target->Active)
				{
					if (item->ObjectNumber == ID_WRAITH1 && target->ObjectNumber == ID_WRAITH2 ||
						item->ObjectNumber == ID_WRAITH2 && target->ObjectNumber == ID_WRAITH1 ||
						item->ObjectNumber == ID_WRAITH3 && target->ObjectNumber == ID_ANIMATING10)
					{
						break;
					}
				}
			}

			if (linkNumber != NO_ITEM)
				item->ItemFlags[6] = linkNumber;
		}

		if (item->ObjectNumber != ID_WRAITH3)
		{
			// WRAITH1 AND WRAITH2 can die on contact with water
			// WRAITH1 dies because it's fire and it dies on contact with water, WRAITH2 instead triggers a flipmap for making icy water
			if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
			{
				TriggerExplosionSparks(item->Position.xPos, item->Position.yPos, item->Position.zPos, 2, -2, 1, item->RoomNumber);
				item->ItemFlags[1]--;
				if (item->ItemFlags[1] < -1)
				{
					if (item->ItemFlags[1] < 30)
					{
						if (item->ObjectNumber == ID_WRAITH2)
						{
							if (item->TriggerFlags)
							{
								if (!FlipStats[item->TriggerFlags])
								{
									DoFlipMap(item->TriggerFlags);
									FlipStats[item->TriggerFlags] = 1;
								}
							}
						}

						KillItem(itemNumber);
					}
				}
				else
					item->ItemFlags[1] = -1;
			}
			else
			{
				item->ItemFlags[1]--;
				if (item->ItemFlags[1] < 0)
					item->ItemFlags[1] = 0;
			}
		}

		if (distance >= 28900 || abs(item->Position.yPos - target->Position.yPos + CLICK(1.5f)) >= CLICK(1))
		{
			if (Wibble & 16)
			{
				if (item->VerticalVelocity < WraithVelocity)
					item->VerticalVelocity++;
				
				if (item->ItemFlags[6])
				{
					if (item->ItemFlags[7])
						item->ItemFlags[7]--;
				}
			}
		}
		else
		{
			if (item->VerticalVelocity > 32)
				item->VerticalVelocity -= 12;
			
			if (target == LaraItem)
			{
				target->HitPoints -= distance / SECTOR(1);

				// WRAITH1 can burn Lara
				if (item->ObjectNumber == ID_WRAITH1)
				{
					item->ItemFlags[1] += 400;
					if (item->ItemFlags[1] > 8000)
						LaraBurn(LaraItem);
				}
			}
			else if (target->ObjectNumber == ID_ANIMATING10)
			{
				// ANIMATING10 is the sacred pedistal that can kill WRAITH
				item->ItemFlags[7]++;
				if (item->ItemFlags[7] > 10)
				{
					item->Position.xPos = target->Position.xPos;
					item->Position.yPos = target->Position.yPos - 384;
					item->Position.zPos = target->Position.zPos;
					WraithExplosionEffect(item, 96, 96, 96, -32);
					WraithExplosionEffect(item, 48, 48, 48, 48);

					target->TriggerFlags--;
					target->HitPoints = 0;

					if (target->TriggerFlags > 0)
						target->FrameNumber = g_Level.Anims[target->AnimNumber].frameBase;

					KillItem(itemNumber);
				}
			}
			else
			{
				// Target is another WRAITH (fire vs ice), they kill both themselves
				target->ItemFlags[7] = target->ItemFlags[7] & 0x6A | 0xA;
				if (item->ItemFlags[7])
				{
					TriggerExplosionSparks(item->Position.xPos, item->Position.yPos, item->Position.zPos, 2, -2, 1, item->RoomNumber);
					target->HitPoints = 0;
					KillItem(item->ItemFlags[6]);
					KillItem(itemNumber);
				}
			}
		}

		// Check if WRAITH is going below floor or above ceiling and trigger sparks
		probe = GetCollisionResult(item);

		if (probe.Position.Floor < item->Position.yPos ||
			probe.Position.Ceiling > item->Position.yPos)
		{
			if (!hitWall)
				WraithWallsEffect(oldX, oldY, oldZ, item->Position.yRot + -ANGLE(180.0f), item->ObjectNumber);
		}
		else if (hitWall)
			WraithWallsEffect(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->Position.yRot, item->ObjectNumber);

		// Update WRAITH nodes
		auto* creature = (WraithInfo*)item->Data;

		int j = 0;
		for (int i = WRAITH_COUNT - 1; i > 0; i--)
		{
			creature[i - 1].Position.xPos += (creature[i - 1].Position.xRot / 16);
			creature[i - 1].Position.yPos += (creature[i - 1].Position.yRot / 16);
			creature[i - 1].Position.zPos += (creature[i - 1].Position.zRot / 16);

			creature[i - 1].Position.xRot -= (creature[i - 1].Position.xRot / 16);
			creature[i - 1].Position.yRot -= (creature[i - 1].Position.yRot / 16);
			creature[i - 1].Position.zRot -= (creature[i - 1].Position.zRot / 16);

			creature[i].Position.xPos = creature[i - 1].Position.xPos;
			creature[i].Position.yPos = creature[i - 1].Position.yPos;
			creature[i].Position.zPos = creature[i - 1].Position.zPos;

			creature[i].Position.xRot = creature[i - 1].Position.xRot;
			creature[i].Position.yRot = creature[i - 1].Position.yRot;
			creature[i].Position.zRot = creature[i - 1].Position.zRot;

			if (item->ObjectNumber == ID_WRAITH1)
			{
				creature[i].r = (GetRandomControl() & 0x3F) - 64;
				creature[i].g = 16 * (j + 1) + (GetRandomControl() & 0x3F);
				creature[i].b = GetRandomControl() & 0xF;
			}
			else if (item->ObjectNumber == ID_WRAITH2)
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

		creature[0].Position.xPos = item->Position.xPos;
		creature[0].Position.yPos = item->Position.yPos;
		creature[0].Position.zPos = item->Position.zPos;

		creature[0].Position.xRot = 4 * (item->Position.xPos - oldX);
		creature[0].Position.yRot = 4 * (item->Position.yPos - oldY);
		creature[0].Position.zRot = 4 * (item->Position.zPos - oldZ);

		// Standard WRAITH drawing code
		DrawWraith(
			item->Position.xPos,
			item->Position.yPos,
			item->Position.zPos,
			creature[0].Position.xRot,
			creature[0].Position.yRot,
			creature[0].Position.zRot,
			item->ObjectNumber);

		DrawWraith(
			(oldX + item->Position.xPos) / 2,
			(oldY + item->Position.yPos) / 2,
			(oldZ + item->Position.zPos) / 2,
			creature[0].Position.xRot,
			creature[0].Position.yRot,
			creature[0].Position.zRot,
			item->ObjectNumber);

		// Lighting for WRAITH
		byte r, g, b;
		if (item->ObjectNumber == ID_WRAITH3)
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
			creature[0].Position.xPos,
			creature[0].Position.yPos,
			creature[0].Position.zPos,
			16,
			r, g, b);
	}

	void WraithExplosionEffect(ITEM_INFO* item, byte r, byte g, byte b, int speed)
	{
		short inner = speed >= 0 ? 32 : 640;
		short outer = speed >= 0 ? 160 : 512;

		item->Position.yPos -= 384;

		TriggerShockwave(&item->Position, inner, outer, speed, r, g, b, 24, 0, 0);
		TriggerShockwave(&item->Position, inner, outer, speed, r, g, b, 24, ANGLE(45.0f), 0);
		TriggerShockwave(&item->Position, inner, outer, speed, r, g, b, 24, ANGLE(90.0f), 0);
		TriggerShockwave(&item->Position, inner, outer, speed, r, g, b, 24, ANGLE(135.0f), 0);

		item->Position.yPos += 384;
	}

	void DrawWraith(int x, int y, int z, short xVelocity, short yVelocity, short zVelocity, int objectNumber)
	{
		auto* spark = &Sparks[GetFreeSpark()];
		spark->on = 1;

		BYTE color;

		if (objectNumber == ID_WRAITH1)
		{
			spark->sR = (GetRandomControl() & 0x1F) + -128;
			spark->sB = 24;
			spark->sG = (GetRandomControl() & 0x1F) + 48;
			spark->dR = (GetRandomControl() & 0x1F) + -128;
			spark->dB = 24;
			spark->dG = (GetRandomControl() & 0x1F) + 64;
		}
		else if (objectNumber == ID_WRAITH2)
		{
			spark->sB = (GetRandomControl() & 0x1F) + -128;
			spark->sR = 24;
			spark->sG = (GetRandomControl() & 0x1F) + -128;
			spark->dB = (GetRandomControl() & 0x1F) + -128;
			spark->dR = 24;
			spark->dG = (GetRandomControl() & 0x1F) + 64;
		}
		else
		{
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
		unsigned char life = (GetRandomControl() & 7) + 12;
		spark->life = life;
		spark->sLife = life;
		spark->x = (GetRandomControl() & 0x1F) + x - 16;
		spark->y = y;
		spark->friction = 85;
		spark->flags = SP_EXPDEF | SP_DEF | SP_SCALE;
		spark->z = (GetRandomControl() & 0x1F) + z - 16;
		spark->xVel = xVelocity;
		spark->yVel = yVelocity;
		spark->zVel = zVelocity;
		spark->gravity = 0;
		spark->maxYvel = 0;
		spark->scalar = 2;
		spark->dSize = 2;
		unsigned char size = (GetRandomControl() & 0x1F) + 48;
		spark->sSize = size;
		spark->size = size;
	}

	void WraithWallsEffect(int x, int y, int z, short yRot, short objectNumber)
	{
		byte sR, sG, sB, dR, dG, dB;
		short color;

		if (objectNumber == ID_WRAITH1)
		{
			sR = (GetRandomControl() & 0x1F) + -128;
			sB = 24;
			sG = (GetRandomControl() & 0x1F) + 48;
			dR = (GetRandomControl() & 0x1F) + -128;
			dB = 24;
			dG = (GetRandomControl() & 0x1F) + 64;
		}
		else if (objectNumber == ID_WRAITH2) {
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
			auto* spark = &Sparks[GetFreeSpark()];

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
			short rot = yRot + GetRandomControl() - ANGLE(90);
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
		ITEM_INFO* item2 = nullptr;

		if (NextItemActive != NO_ITEM)
		{
			for (; NextItemActive != NO_ITEM;)
			{
				auto* item2 = &g_Level.Items[NextItemActive];
				if (item2->ObjectNumber == ID_WRAITH3 && !item2->HitPoints)
					break;
				
				if (item2->NextActive == NO_ITEM)
				{
					FlipEffect = -1;
					return;
				}
			}

			item2->HitPoints = item - g_Level.Items.data();
		}

		FlipEffect = -1;
	}
}
