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

		item->Animation.Velocity = WraithVelocity;
		item->ItemFlags[0] = 0;
		item->ItemFlags[6] = 0;

		for (int i = 0; i < WRAITH_COUNT; i++)
		{
			wraith->Position = Vector3Int(0, 0, item->Pose.Position.z);
			wraith->Velocity.z = 0;
			wraith->r = 0;
			wraith->g = 0;
			wraith->b = 0;

			wraith++;
		}
	}

	void WraithControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		SoundEffect(SFX_TR4_WRAITH_WHISPERS, &item->Pose);

		// HitPoints stores the target of wraith
		auto* target = item->ItemFlags[6] ? &g_Level.Items[item->ItemFlags[6]] : LaraItem;

		auto oldPos = item->Pose.Position;

		int x, y, z;
		int dy;
		int distance;

		if (target == LaraItem || target->ObjectNumber == ID_ANIMATING10)
		{
			x = target->Pose.Position.x - item->Pose.Position.x;
			y = target->Pose.Position.y;
			z = target->Pose.Position.z - item->Pose.Position.z;
			distance = pow(x, 2) + pow(z, 2);
			dy = abs((distance / SECTOR(8)) - SECTOR(0.5f));
		}
		else
		{
			auto* room = &g_Level.Rooms[LaraItem->RoomNumber];

			x = room->x + room->xSize * SECTOR(1) / 2 - item->Pose.Position.x;
			z = room->z + room->zSize * SECTOR(1) / 2 - item->Pose.Position.z;

			distance = pow(x, 2) + pow(z, 2);
			dy = abs((distance / MAX_VISIBILITY_DISTANCE) - CLICK(1));
			y = room->y + ((room->minfloor - room->maxceiling) / 2);
		}

		dy = y - item->Pose.Position.y - dy - CLICK(0.5f);
		short angleH = phd_atan(z, x) - item->Pose.Orientation.y;

		short angleV = 0;
		if (abs(x) <= abs(z))
			angleV = phd_atan(abs(x) + (abs(z) / 2), dy);
		else
			angleV = phd_atan(abs(z) + (abs(x) / 2), dy);

		angleV -= item->Pose.Orientation.x;

		int velocity = (WraithVelocity / item->Animation.Velocity) * 8;

		if (abs(angleH) >= item->ItemFlags[2] || angleH > 0 != item->ItemFlags[2] > 0)
		{
			if (angleH >= 0)
			{
				if (item->ItemFlags[2] <= 0)
					item->ItemFlags[2] = 1;
				else
				{
					item->ItemFlags[2] += velocity;
					item->Pose.Orientation.y += item->ItemFlags[2];
				}
			}
			else if (item->ItemFlags[2] >= 0)
				item->ItemFlags[2] = -1;
			else
			{
				item->ItemFlags[2] -= velocity;
				item->Pose.Orientation.y += item->ItemFlags[2];
			}
		}
		else
			item->Pose.Orientation.y += angleH;

		if (abs(angleV) >= item->ItemFlags[3] || angleV > 0 != item->ItemFlags[3] > 0)
		{
			if (angleV >= 0)
			{
				if (item->ItemFlags[3] <= 0)
					item->ItemFlags[3] = 1;
				else
				{
					item->ItemFlags[3] += velocity;
					item->Pose.Orientation.x += item->ItemFlags[3];
				}
			}
			else if (item->ItemFlags[3] >= 0)
				item->ItemFlags[3] = -1;
			else
			{
				item->ItemFlags[3] -= velocity;
				item->Pose.Orientation.x += item->ItemFlags[3];
			}
		}
		else
			item->Pose.Orientation.x += angleV;

		auto probe = GetCollision(item);

		bool hitWall = false;
		if (probe.Position.Floor < item->Pose.Position.y || probe.Position.Ceiling > item->Pose.Position.y)
			hitWall = true;

		item->Pose.Position.x += item->Animation.Velocity * phd_sin(item->Pose.Orientation.y);
		item->Pose.Position.y += item->Animation.Velocity * phd_sin(item->Pose.Orientation.x);
		item->Pose.Position.z += item->Animation.Velocity * phd_cos(item->Pose.Orientation.y);

		auto outsideRoom = IsRoomOutside(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
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
			if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
			{
				TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 2, -2, 1, item->RoomNumber);
				
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

		if (distance >= SECTOR(28.25f) ||
			(abs(item->Pose.Position.y - target->Pose.Position.y + CLICK(1.5f))) >= CLICK(1))
		{
			if (Wibble & 16)
			{
				if (item->Animation.Velocity < WraithVelocity)
					item->Animation.Velocity++;
				
				if (item->ItemFlags[6])
				{
					if (item->ItemFlags[7])
						item->ItemFlags[7]--;
				}
			}
		}
		else
		{
			if (item->Animation.Velocity > 32)
				item->Animation.Velocity -= 12;
			
			if (target->IsLara())
			{
				DoDamage(target, distance / SECTOR(1));

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
					item->Pose.Position = target->Pose.Position;
					item->Pose.Position.y -= CLICK(1.5f);

					WraithExplosionEffect(item, 96, 96, 96, -32);
					WraithExplosionEffect(item, 48, 48, 48, 48);

					target->TriggerFlags--;

					if (target->TriggerFlags > 0)
						target->Animation.FrameNumber = g_Level.Anims[target->Animation.AnimNumber].frameBase;

					DoDamage(target, INT_MAX);
					KillItem(itemNumber);
				}
			}
			else
			{
				// Target is another WRAITH (fire vs ice), they kill both themselves
				target->ItemFlags[7] = target->ItemFlags[7] & 0x6A | 0xA;
				if (item->ItemFlags[7])
				{
					TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 2, -2, 1, item->RoomNumber);
					DoDamage(target, INT_MAX);
					KillItem(item->ItemFlags[6]);
					KillItem(itemNumber);
				}
			}
		}

		// Check if WRAITH is going below floor or above ceiling and trigger sparks
		probe = GetCollision(item);

		if (probe.Position.Floor < item->Pose.Position.y ||
			probe.Position.Ceiling > item->Pose.Position.y)
		{
			if (!hitWall)
				WraithWallsEffect(oldPos, item->Pose.Orientation.y - ANGLE(180.0f), item->ObjectNumber);
		}
		else if (hitWall)
			WraithWallsEffect(item->Pose.Position, item->Pose.Orientation.y, item->ObjectNumber);

		// Update WRAITH nodes
		auto* wraith = (WraithInfo*)item->Data;

		int j = 0;
		for (int i = WRAITH_COUNT - 1; i > 0; i--)
		{
			wraith[i - 1].Position += (wraith[i - 1].Velocity / 16);
			wraith[i - 1].Velocity -= (wraith[i - 1].Velocity / 16);

			wraith[i].Position = wraith[i - 1].Position;
			wraith[i].Velocity = wraith[i - 1].Velocity;

			if (item->ObjectNumber == ID_WRAITH1)
			{
				wraith[i].r = (GetRandomControl() & 0x3F) - 64;
				wraith[i].g = 16 * (j + 1) + (GetRandomControl() & 0x3F);
				wraith[i].b = GetRandomControl() & 0xF;
			}
			else if (item->ObjectNumber == ID_WRAITH2)
			{
				wraith[i].r = GetRandomControl() & 0xF;
				wraith[i].g = 16 * (j + 1) + (GetRandomControl() & 0x3F);
				wraith[i].b = (GetRandomControl() & 0x3F) - 64;
			}
			else
			{
				wraith[i].r = 8 * (j + 2) + (GetRandomControl() & 0x3F);
				wraith[i].g = wraith[i].r;
				wraith[i].b = wraith[i].r + (GetRandomControl() & 0xF);
			}

			j++;
		}

		wraith[0].Position = item->Pose.Position;
		wraith[0].Velocity = (item->Pose.Position - oldPos) * 4;

		// Standard WRAITH drawing code
		DrawWraith(
			item->Pose.Position,
			wraith[0].Velocity,
			item->ObjectNumber);

		DrawWraith(
			(oldPos + item->Pose.Position) / 2,
			wraith[0].Velocity,
			item->ObjectNumber);

		// Lighting for WRAITH
		byte r, g, b;
		if (item->ObjectNumber == ID_WRAITH3)
		{
			r = wraith[5].r;
			g = wraith[5].g;
			b = wraith[5].b;
		}
		else
		{
			r = wraith[1].r;
			g = wraith[1].g;
			b = wraith[1].b;
		}

		TriggerDynamicLight(
			wraith[0].Position.x,
			wraith[0].Position.y,
			wraith[0].Position.z,
			16,
			r, g, b);
	}

	void WraithExplosionEffect(ItemInfo* item, byte r, byte g, byte b, int speed)
	{
		short inner = speed >= 0 ? 32 : 640;
		short outer = speed >= 0 ? 160 : 512;

		item->Pose.Position.y -= 384;

		TriggerShockwave(&item->Pose, inner, outer, speed, r, g, b, 24, 0, 0);
		TriggerShockwave(&item->Pose, inner, outer, speed, r, g, b, 24, ANGLE(45.0f), 0);
		TriggerShockwave(&item->Pose, inner, outer, speed, r, g, b, 24, ANGLE(90.0f), 0);
		TriggerShockwave(&item->Pose, inner, outer, speed, r, g, b, 24, ANGLE(135.0f), 0);

		item->Pose.Position.y += 384;
	}

	void DrawWraith(Vector3Int pos, Vector3Int velocity, int objectNumber)
	{
		auto* spark = GetFreeParticle();
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
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		unsigned char life = (GetRandomControl() & 7) + 12;
		spark->life = life;
		spark->sLife = life;
		spark->x = (GetRandomControl() & 0x1F) + pos.x - 16;
		spark->y = pos.y;
		spark->friction = 85;
		spark->flags = SP_EXPDEF | SP_DEF | SP_SCALE;
		spark->z = (GetRandomControl() & 0x1F) + pos.z - 16;
		spark->xVel = velocity.x;
		spark->yVel = velocity.y;
		spark->zVel = velocity.z;
		spark->gravity = 0;
		spark->maxYvel = 0;
		spark->scalar = 2;
		spark->dSize = 2;
		unsigned char size = (GetRandomControl() & 0x1F) + 48;
		spark->sSize = size;
		spark->size = size;
	}

	void WraithWallsEffect(Vector3Int pos, short yRot, short objectNumber)
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
			auto* spark = GetFreeParticle();

			spark->on = true;
			spark->sR = dR;
			spark->sG = dG;
			spark->sB = dB;
			spark->dR = dR;
			spark->dG = dG;
			spark->dB = dB;
			spark->colFadeSpeed = 4;
			spark->fadeToBlack = 7;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			short life = (GetRandomControl() & 7) + 32;
			spark->life = life;
			spark->sLife = life;
			spark->x = (GetRandomControl() & 0x1F) + pos.x - 16;
			spark->y = (GetRandomControl() & 0x1F) + pos.y - 16;
			spark->z = (GetRandomControl() & 0x1F) + pos.z - 16;
			short rot = yRot + GetRandomControl() - ANGLE(90.0f);
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

	void KillWraith(ItemInfo* item)
	{
		ItemInfo* item2 = nullptr;

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
