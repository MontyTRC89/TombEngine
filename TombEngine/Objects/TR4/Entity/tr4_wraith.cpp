#include "framework.h"
#include "Objects/TR4/Entity/tr4_wraith.h"

#include "Game/collision/collide_room.h"
#include "Game/control/flipeffect.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/Streamer.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Game/room.h"
#include "Math/Math.h"
#include "Objects/Generic/Traps/traps.h"
#include "Objects/TR4/Entity/tr4_wraith_info.h"
#include "Objects/objectslist.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Math/Geometry.h"

using namespace TEN::Effects::Items;
using namespace TEN::Effects::Streamer;
using namespace TEN::Math;
using namespace TEN::Math::Geometry;

namespace TEN::Entities::TR4
{
	constexpr auto WRAITH_COUNT	   = 8;
	constexpr auto WRAITH_VELOCITY = 64;
	constexpr auto WRAITH_TAIL_OFFSET = Vector3(0, -10, -50);
	constexpr auto MAX_DISTANCE_TO_WRAITH_TRAP = SQUARE(BLOCK(2));

	void InitialiseWraith(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->Data = WraithInfo();
		auto* wraith = (WraithInfo*)item->Data;

		item->Animation.Velocity.z = WRAITH_VELOCITY;
		item->ItemFlags[0] = 0;
		item->ItemFlags[6] = 0;

		for (int i = 0; i < WRAITH_COUNT; i++)
		{
			wraith->Position = Vector3i(0, 0, item->Pose.Position.z);
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
		
		// HACK: HitPoints stores the wraith's target.	
		auto* target = item->ItemFlags[6] ? &g_Level.Items[item->ItemFlags[6]] : LaraItem;

		auto prevPos = item->Pose.Position;
		int x, y, z, xl, yl, zl;
		int dy;
		int distance, distanceLara;

		if (target->IsLara() || target->ObjectNumber == ID_WRAITH_TRAP)
		{
			x = target->Pose.Position.x - item->Pose.Position.x;
			y = target->Pose.Position.y;
			z = target->Pose.Position.z - item->Pose.Position.z;
			distance = SQUARE(x) + SQUARE(z);
			dy = abs((distance / SECTOR(8)) - SECTOR(0.5f));
		}
		else
		{
			auto* room = &g_Level.Rooms[LaraItem->RoomNumber];

			x = room->x + room->xSize * SECTOR(1) / 2 - item->Pose.Position.x;
			z = room->z + room->zSize * SECTOR(1) / 2 - item->Pose.Position.z;

			distance = SQUARE(x) + SQUARE(z);
			dy = abs((distance / MAX_VISIBILITY_DISTANCE) - CLICK(1));
			//Prevent Wraiths to go below floor level
			y = room->y + ((room->maxceiling - room->minfloor) / 4);
		}

		dy = y - item->Pose.Position.y - dy - CLICK(0.5f);
		short angleH = phd_atan(z, x) - item->Pose.Orientation.y;

		short angleV = 0;
		if (abs(x) <= abs(z))
			angleV = phd_atan(abs(x) + (abs(z)), dy);
		else
			angleV = phd_atan(abs(z) + (abs(x)), dy);

		angleV -= item->Pose.Orientation.x;
		int velocity = (WRAITH_VELOCITY / item->Animation.Velocity.z) * 8;

		if (abs(angleH) < item->ItemFlags[2] && angleH > 0 == item->ItemFlags[2] > 0)
		{
			item->Pose.Orientation.y += angleH;
		}
		else if (angleH >= 0)
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
		
		item->Pose.Position.x += item->Animation.Velocity.z * phd_sin(item->Pose.Orientation.y);
		item->Pose.Position.y += item->Animation.Velocity.z * phd_sin(item->Pose.Orientation.x);
		item->Pose.Position.z += item->Animation.Velocity.z * phd_cos(item->Pose.Orientation.y);

		if (probe.RoomNumber != item->RoomNumber)
		{

			ItemNewRoom(itemNumber, probe.RoomNumber);

			for (int linkNumber = g_Level.Rooms[item->RoomNumber].itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
			{
				auto* targetItem = &g_Level.Items[linkNumber];

				if (targetItem->Active)
				{
					if (item->ObjectNumber == ID_WRAITH1 && targetItem->ObjectNumber == ID_WRAITH2 ||
						item->ObjectNumber == ID_WRAITH2 && targetItem->ObjectNumber == ID_WRAITH1 ||
						item->ObjectNumber == ID_WRAITH3 && targetItem->ObjectNumber == ID_WRAITH_TRAP)
					{
						if (item->ObjectNumber == ID_WRAITH3 && targetItem->ObjectNumber == ID_WRAITH_TRAP)
						{
							x = targetItem->Pose.Position.x - item->Pose.Position.x;
							y = targetItem->Pose.Position.y;
							z = targetItem->Pose.Position.z - item->Pose.Position.z;
							distance = SQUARE(x) + SQUARE(z);

							xl = targetItem->Pose.Position.x - LaraItem->Pose.Position.x;
							yl = targetItem->Pose.Position.y;
							zl = targetItem->Pose.Position.z - LaraItem->Pose.Position.z;
							distanceLara = SQUARE(xl) + SQUARE(zl);

							//Wraith 3 attacks the wraith trap only if it is close to the trap and if lara is 1 Block close to the trap too
							if (distance < MAX_DISTANCE_TO_WRAITH_TRAP && distanceLara < MAX_DISTANCE_TO_WRAITH_TRAP)
							{
								item->ItemFlags[6] = linkNumber;
								targetItem->ItemFlags[6] = 1;
							}
							break;
						}
						else
						{
								item->ItemFlags[6] = linkNumber;
								targetItem->ItemFlags[6] = 1;
						}
					}
				}					
			}		
		}

		if (item->ObjectNumber != ID_WRAITH3)
		{
			// WRAITH1 AND WRAITH2 can die on contact with water
			// WRAITH1 dies because it's fire and it dies on contact with water, WRAITH2 instead triggers a flipmap for making icy waterp
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
		else
		{	//Make some fancy effects when wraith3 gets sucked in from ghost trap
			if (target->ObjectNumber == ID_WRAITH_TRAP)
			{

				xl = target->Pose.Position.x - LaraItem->Pose.Position.x;
				yl = target->Pose.Position.y;
				zl = target->Pose.Position.z - LaraItem->Pose.Position.z;
				distanceLara = SQUARE(xl) + SQUARE(zl);

				//Wraith3 can escape from the trap if it is not close to the trap and if lara is 2 Block away from the trap
				if (distance < MAX_DISTANCE_TO_WRAITH_TRAP && distanceLara < MAX_DISTANCE_TO_WRAITH_TRAP)
				{
					if (target->TriggerFlags > 0)
					{
						Vector3i lightningStart = item->Pose.Position;
						GameVector lightningEnd = GetJointPosition(target, 0, Vector3i::Zero);

						int amplitude = (GetRandomControl() & 1) + 15;

						SoundEffect(SFX_TR4_ELECTRIC_ARCING_LOOP, &Pose(Vector3i(lightningStart)));

						SpawnElectricity(lightningStart.ToVector3(), lightningEnd.ToVector3(), amplitude, 255, 255, 255, 10, (int)ElectricityFlags::ThinIn, 12, 10);
						SpawnElectricity(lightningStart.ToVector3(), lightningEnd.ToVector3(), amplitude, 255, 255, 255, 10, (int)ElectricityFlags::ThinIn, 4, 10);
						SpawnElectricity(lightningStart.ToVector3(), lightningEnd.ToVector3(), amplitude, 255, 100, 0, 10, (int)ElectricityFlags::ThinIn, 3, 10);

						//Trigger attack sparks on WraithTrap
						target->ItemFlags[6] = 1;
					}
				}
				else
				{
					item->ItemFlags[6] = 0;
					target->ItemFlags[6] = 0;
					target = LaraItem;
				}		
			}
		}

		if (distance < SECTOR(28.25f) &&
			(abs(item->Pose.Position.y - target->Pose.Position.y + CLICK(1.5f))) < CLICK(1))
		{
			if (item->Animation.Velocity.z > 32)
				item->Animation.Velocity.z -= 12;

			if (target->IsLara())
			{
				DoDamage(target, distance / SECTOR(1));

				// WRAITH1 can burn Lara
				if (item->ObjectNumber == ID_WRAITH1)
				{
					item->ItemFlags[1] += 400;
					if (item->ItemFlags[1] > 8000)
						ItemBurn(LaraItem);
				}
			}
			else if (target->ObjectNumber == ID_WRAITH_TRAP)
			{
				// ID_WRAITH_TRAP can kill WRAITH3
				item->ItemFlags[7]++;

				if (item->ItemFlags[7] > 10)
				{
					item->Pose.Position = target->Pose.Position;
					item->Pose.Position.y -= CLICK(1.5f);

					WraithExplosionEffect(item, 96, 96, 96, -32);
					WraithExplosionEffect(item, 48, 48, 48, 48);

					//target->TriggerFlags--;

					if (target->TriggerFlags > 0)
						target->Animation.FrameNumber = g_Level.Anims[target->Animation.AnimNumber].frameBase;

					target->ItemFlags[6] = 0;
					DoDamage(target, INT_MAX);
					KillItem(itemNumber);
				}
			}
			else
			{
				// Target is another WRAITH (fire vs ice), they kill both themselves

				target->ItemFlags[7] = 10;

				if (item->ItemFlags[7])
				{
					if (item->ObjectNumber == ID_WRAITH1)
						WraithExplosionEffect(item, 1.0f * UCHAR_MAX, 0.6f * UCHAR_MAX, 0.0f * UCHAR_MAX, 48);					
					else
						WraithExplosionEffect(item, 0.0f * UCHAR_MAX, 0.5f * UCHAR_MAX, 1.0f * UCHAR_MAX, 48);					

					TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 2, -2, 1, item->RoomNumber);
					DoDamage(item, INT_MAX);
					item->ItemFlags[6] = 0;
					target->ItemFlags[6] = 0;
					KillItem(item->ItemFlags[6]);
					KillItem(itemNumber);
				}
			}
		}
		else
		{
			if (Wibble & 10)
			{
				if (item->Animation.Velocity.z < WRAITH_VELOCITY)
					item->Animation.Velocity.z++;

				if (item->ItemFlags[6])
				{
						target->ItemFlags[7]--;
				}
			}						
		}

		// Check if WRAITH is going below floor or above ceiling and trigger sparks
		probe = GetCollision(item);

		if (probe.Position.Floor < item->Pose.Position.y ||
			probe.Position.Ceiling > item->Pose.Position.y)
		{
			if (!hitWall)
				WraithWallsEffect(prevPos, item->Pose.Orientation.y - ANGLE(180.0f), item->ObjectNumber);
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
		wraith[0].Velocity = (item->Pose.Position - prevPos) * 4;

		// Standard WRAITH drawing code
		DrawWraith(
			item->Pose.Position,
			wraith[0].Velocity,
			item->ObjectNumber);

		DrawWraith(
			(prevPos + item->Pose.Position) / 2,
			wraith[0].Velocity,
			item->ObjectNumber);

		SpawnWraithTails(*item, WRAITH_TAIL_OFFSET, item->ObjectNumber);

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

		TriggerShockwave(&item->Pose, inner, outer, speed, r, g, b, 24, EulerAngles::Zero, 0, true, false, (int)ShockwaveStyle::Normal);
		TriggerShockwave(&item->Pose, inner, outer, speed, r, g, b, 24, EulerAngles(ANGLE(45.0f), 0.0f, 0.0f), 0, true, false, (int)ShockwaveStyle::Normal);
		TriggerShockwave(&item->Pose, inner, outer, speed, r, g, b, 24, EulerAngles(ANGLE(90.0f), 0.0f, 0.0f), 0, true, false, (int)ShockwaveStyle::Normal);
		TriggerShockwave(&item->Pose, inner, outer, speed, r, g, b, 24, EulerAngles(ANGLE(135.0f), 0.0f, 0.0f), 0, true, false, (int)ShockwaveStyle::Normal);
	}

	void DrawWraith(Vector3i pos, Vector3i velocity, int objectNumber)
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

	void WraithWallsEffect(Vector3i pos, short yRot, int objectNumber)
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
		else if (objectNumber == ID_WRAITH2)
		{
			sB = (GetRandomControl() & 0x1F) + -128;
			sR = 24;
			sG = (GetRandomControl() & 0x1F) + -128;
			dB = (GetRandomControl() & 0x1F) + -128;
			dR = 24;
			dG = (GetRandomControl() & 0x1F) + 64;
		}
		else
		{
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

	void SpawnWraithTails(const ItemInfo& wraithItem, const Vector3& relOffset, int objectNumber)
	{
		auto COLOR = Vector4::Zero;
		
		switch (objectNumber)
		{
		case ID_WRAITH1:
			COLOR = Vector4(1.0f, 0.6f, 0.0f, 1.0f);
			break;
		case ID_WRAITH2:
			COLOR = Vector4(0.0f, 0.5f, 1.0f, 1.0f);
			break;
		case ID_WRAITH3:
			COLOR = Vector4(1.0f);
			break;
		}

		constexpr auto LIFE_MAX = 0.5f;
		constexpr auto VEL_ABS = 4.0f;
		constexpr auto SCALE_RATE = 1.0f;
		auto posBase = Vector3(wraithItem.Pose.Position.x, wraithItem.Pose.Position.y, wraithItem.Pose.Position.z);
		auto orient = EulerAngles(wraithItem.Pose.Orientation.x, wraithItem.Pose.Orientation.y, wraithItem.Pose.Orientation.z);
		auto rotMatrix = orient.ToRotationMatrix();

		auto startOffset =  Vector3(relOffset.x, relOffset.y, relOffset.z);
		auto startPos = posBase + Vector3::Transform(startOffset, rotMatrix);

		auto directionL = Geometry::RotatePoint(posBase, EulerAngles(ANGLE(50.0f), 0, 0));
		auto directionR = Geometry::RotatePoint(posBase, EulerAngles(ANGLE(-50.0f), 0, 0));
		auto directionD = Geometry::RotatePoint(posBase, EulerAngles(0, ANGLE(50.0f), 0));

		short orient2D = wraithItem.Pose.Orientation.z;

		float life = LIFE_MAX;
		float vel = VEL_ABS ;

		// Spawn left tail.
		StreamerEffect.Spawn(
			wraithItem.Index, 0,
			startPos, directionL, orient2D, COLOR,
			8.0f, life, vel, SCALE_RATE, 0, 2);

		// Spawn right tail.
		StreamerEffect.Spawn(
			wraithItem.Index, 1,
			startPos, directionR, orient2D, COLOR,
			8.0f, life, vel, SCALE_RATE, 0, 2);

		// Spawn third tail.
		StreamerEffect.Spawn(
			wraithItem.Index, 2,
			startPos, directionD, orient2D, COLOR,
			8.0f, life, vel, SCALE_RATE, 0, 2);
	}


}
