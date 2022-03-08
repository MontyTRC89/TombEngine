#include "framework.h"
#include "Objects/Effects/flame_emitters.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/lightning.h"
#include "Game/effects/lara_fx.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"

using namespace TEN::Effects::Lara;
using namespace TEN::Effects::Lightning;
using namespace TEN::Effects::Environment;

namespace TEN::Entities::Effects
{
	byte Flame3xzoffs[16][2] = {{ 9, 9 },
								{ 24, 9 },
								{ 40, 9	},
								{ 55, 9 },
								{ 9, 24 },
								{ 24, 24 },
								{ 40, 24 },
								{ 55, 24 },
								{ 9, 40 },
								{ 24, 40 },
								{ 40, 40 },
								{ 55, 40 },
								{ 9, 55	 },
								{ 24, 55 },
								{ 40, 55 },
								{ 55, 55 }
	};

	OBJECT_COLLISION_BOUNDS FireBounds = {
		0, 0, 
		0, 0, 
		0, 0, 
		-ANGLE(10), ANGLE(10), 
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10) 
	};

	bool FlameEmitterFlags[8];

	void FlameEmitterControl(short itemNumber)
	{
		byte r, g, b;
		int falloff;

		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			// Jet flame
			if (item->TriggerFlags < 0)
			{
				short flags = -item->TriggerFlags;
				if ((flags & 7) == 2 || (flags & 7) == 7)
				{
					SoundEffect(SFX_TR4_FLAME_EMITTER, &item->Position, 0);
					TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
					TriggerDynamicLight(item->Position.xPos, item->Position.yPos, item->Position.zPos,
						(GetRandomControl() & 3) + 20,
						(GetRandomControl() & 0x3F) + 192,
						(GetRandomControl() & 0x1F) + 96, 0);
				}
				else
				{
					if (item->ItemFlags[0])
					{
						if (item->ItemFlags[1])
							item->ItemFlags[1] = item->ItemFlags[1] - (item->ItemFlags[1] >> 2);

						if (item->ItemFlags[2] < 256)
							item->ItemFlags[2] += 8;

						item->ItemFlags[0]--;
						if (item->ItemFlags[0] == 1)
							item->ItemFlags[3] = (GetRandomControl() & 0x3F) + 150;
					}
					else
					{
						if (!--item->ItemFlags[3])
						{
							if (flags >> 3)
								item->ItemFlags[0] = (GetRandomControl() & 0x1F) + 30 * (flags >> 3);
							else
								item->ItemFlags[0] = (GetRandomControl() & 0x3F) + 60;
						}

						if (item->ItemFlags[2])
							item->ItemFlags[2] -= 8;

						if (item->ItemFlags[1] > -8192)
							item->ItemFlags[1] -= 512;
					}

					if (item->ItemFlags[2])
						AddFire(item->Position.xPos, item->Position.yPos, item->Position.zPos, SP_NORMALFIRE, item->RoomNumber, item->ItemFlags[2]);

					if (item->ItemFlags[1])
					{
						SoundEffect(SFX_TR4_FLAME_EMITTER, &item->Position, 0);

						if (item->ItemFlags[1] <= -8192)
							TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
						else
							TriggerSuperJetFlame(item, item->ItemFlags[1], GlobalCounter & 1);

						TriggerDynamicLight(item->Position.xPos, item->Position.yPos, item->Position.zPos,
							(-item->ItemFlags[1] >> 10) - (GetRandomControl() & 1) + 16,
							(GetRandomControl() & 0x3F) + 192,
							(GetRandomControl() & 0x1F) + 96, 0);
					}
					else
					{
						r = (GetRandomControl() & 0x3F) + 192;
						g = (GetRandomControl() & 0x1F) + 96;
						falloff = 10 - (GetRandomControl() & 1);
						TriggerDynamicLight(item->Position.xPos, item->Position.yPos, item->Position.zPos,
							10 - (GetRandomControl() & 1),
							(GetRandomControl() & 0x3F) + 192,
							(GetRandomControl() & 0x1F) + 96, 0);
					}
				}

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Position, 0);
			}
			else
			{
				if (item->TriggerFlags < 8)
					FlameEmitterFlags[item->TriggerFlags] = true;

				AddFire(item->Position.xPos, item->Position.yPos, item->Position.zPos, SP_BIGFIRE, item->RoomNumber, 0);

				TriggerDynamicLight(item->Position.xPos, item->Position.yPos, item->Position.zPos,
					16 - (GetRandomControl() & 1),
					(GetRandomControl() & 0x3F) + 192,
					(GetRandomControl() & 0x1F) + 96, 0);

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Position, 0);

				if (!Lara.burn
					&& ItemNearLara(&item->Position, 600)
					&& (SQUARE(LaraItem->Position.xPos - item->Position.xPos) +
						SQUARE(LaraItem->Position.zPos - item->Position.zPos) < SQUARE(512))
					&& Lara.Control.WaterStatus != WaterStatus::FlyCheat)
				{
					LaraBurn(LaraItem);
				}
			}
		}
		else
		{
			if (item->TriggerFlags > 0 && item->TriggerFlags < 8)
				FlameEmitterFlags[item->TriggerFlags] = false;
		}
	}

	void FlameEmitter2Control(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			// If not an emitter for flipmaps
			if (item->TriggerFlags >= 0)
			{
				// If not a moving flame
				if (item->TriggerFlags != 2)
				{
					if (item->TriggerFlags == 123)
					{
						// Middle of the block
						AddFire(item->Position.xPos, item->Position.yPos, item->Position.zPos, SP_SMALLFIRE, item->RoomNumber, item->ItemFlags[3]);
					}
					else
					{
						AddFire(item->Position.xPos, item->Position.yPos, item->Position.zPos, SP_SMALLFIRE - item->TriggerFlags, item->RoomNumber, item->ItemFlags[3]);
					}
				}

				if (item->TriggerFlags == 0 || item->TriggerFlags == 2)
				{
					if (item->ItemFlags[3])
					{
						TriggerDynamicLight(item->Position.xPos, item->Position.yPos, item->Position.zPos,
							10,
							((GetRandomControl() & 0x3F) + 192) * item->ItemFlags[3] >> 8,
							(GetRandomControl() & 0x1F) + 96 * item->ItemFlags[3] >> 8,
							0);
					}
					else
					{
						TriggerDynamicLight(item->Position.xPos, item->Position.yPos, item->Position.zPos,
							10,
							(GetRandomControl() & 0x3F) + 192,
							(GetRandomControl() & 0x1F) + 96,
							0);
					}
				}

				if (item->TriggerFlags == 2)
				{
					item->Position.xPos += phd_sin(item->Position.yRot - ANGLE(180));
					item->Position.zPos += phd_cos(item->Position.yRot - ANGLE(180));

					short roomNumber = item->RoomNumber;
					FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);

					if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER)
					{
						Weather.Flash(255, 128, 0, 0.03f);
						KillItem(itemNumber);
						return;
					}

					if (item->RoomNumber != roomNumber)
					{
						ItemNewRoom(itemNumber, roomNumber);
					}

					item->Position.yPos = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);;

					if (Wibble & 7)
					{
						TriggerFireFlame(item->Position.xPos, item->Position.yPos - 32, item->Position.zPos, -1, 1);
					}
				}

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Position, 0);
			}
			else if (item->ItemFlags[0] == 0)
			{
				DoFlipMap(-item->TriggerFlags);
				FlipMap[-item->TriggerFlags] ^= 0x3E00u;
				item->ItemFlags[0] = 1;
			}
		}
	}

	void FlameControl(short fxNumber)
	{
		FX_INFO* fx = &EffectList[fxNumber];

		for (int i = 0; i < 14; i++)
		{
			if (!(Wibble & 0xC))
			{
				fx->pos.xPos = 0;
				fx->pos.yPos = 0;
				fx->pos.zPos = 0;

				GetLaraJointPosition((PHD_VECTOR*)&fx->pos, i);

				// TR5 code?
				if (Lara.BurnCount)
				{
					Lara.BurnCount--;
					if (!Lara.BurnCount)
						Lara.BurnSmoke = true;
				}

				TriggerFireFlame(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, -1, 255 - Lara.BurnSmoke);
			}
		}

		byte r = (GetRandomControl() & 0x3F) + 192;
		byte g = (GetRandomControl() & 0x1F) + 96;
		byte b;

		PHD_VECTOR pos{ 0,0,0 };
		GetLaraJointPosition(&pos, LM_HIPS);

		if (!Lara.BurnSmoke)
		{
			if (Lara.burnBlue == 0)
			{
				TriggerDynamicLight(
					pos.x,
					pos.y,
					pos.z,
					13,
					(GetRandomControl() & 0x3F) + 192,
					(GetRandomControl() & 0x1F) + 96,
					0);
			}
			else if (Lara.burnBlue == 1)
			{
				TriggerDynamicLight(
					pos.x, 
					pos.y, 
					pos.z, 
					13, 
					0, 
					(GetRandomControl() & 0x1F) + 96,
					(GetRandomControl() & 0x3F) + 192);
			}
			else if (Lara.burnBlue == 2)
			{
				TriggerDynamicLight(
					pos.x,
					pos.y,
					pos.z,
					13,
					0,
					(GetRandomControl() & 0x3F) + 192,
					(GetRandomControl() & 0x1F) + 96);
			}
		}
		else
		{
			TriggerDynamicLight(
				pos.x,
				pos.y,
				pos.z, 
				13,
				GetRandomControl() & 0x3F,
				(GetRandomControl() & 0x3F) + 192,
				(GetRandomControl() & 0x1F) + 96);
		}

		if (LaraItem->RoomNumber != fx->roomNumber)
			EffectNewRoom(fxNumber, LaraItem->RoomNumber);

		int wh = GetWaterHeight(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, fx->roomNumber);
		if (wh == NO_HEIGHT || fx->pos.yPos <= wh || Lara.burnBlue)
		{
			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &fx->pos, 0);

			LaraItem->HitPoints -= 7;
			LaraItem->HitStatus = true;
		}
		else
		{
			KillEffect(fxNumber);
			Lara.burn = false;
		}

		if (Lara.Control.WaterStatus == WaterStatus::FlyCheat)
		{
			KillEffect(fxNumber);
			Lara.burn = false;
		}
	}

	void InitialiseFlameEmitter(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->TriggerFlags < 0)
		{
			item->ItemFlags[0] = (GetRandomControl() & 0x3F) + 90;
			item->ItemFlags[2] = 256;

			if (((-item->TriggerFlags) & 7) == 7)
			{
				switch (item->Position.yRot)
				{
				case 0:
					item->Position.zPos += 512;
					break;

				case 0x4000:
					item->Position.xPos += 512;
					break;

				case -0x8000:
					item->Position.zPos -= 512;
					break;

				case -0x4000:
					item->Position.xPos -= 512;
					break;
				}
			}
		}
	}

	void InitialiseFlameEmitter2(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		item->Position.yPos -= 64;

		if (item->TriggerFlags != 123)
		{
			switch (item->Position.yRot)
			{
			case 0:
				if (item->TriggerFlags == 2)
					item->Position.zPos += 80;
				else
					item->Position.zPos += 256;
				break;

			case 0x4000:
				if (item->TriggerFlags == 2)
					item->Position.xPos += 80;
				else
					item->Position.xPos += 256;
				break;

			case -0x8000:
				if (item->TriggerFlags == 2)
					item->Position.zPos -= 80;
				else
					item->Position.zPos -= 256;
				break;

			case -0x4000:
				if (item->TriggerFlags == 2)
					item->Position.xPos -= 80;
				else
					item->Position.xPos -= 256;
				break;
			}
		}
	}

	void InitialiseFlameEmitter3(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->TriggerFlags >= 3)
		{
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				ITEM_INFO* currentItem = &g_Level.Items[i];

				if (currentItem->ObjectNumber == ID_ANIMATING3)
				{
					if (currentItem->TriggerFlags == item->TriggerFlags)
						item->ItemFlags[2] = i;
					else if (currentItem->TriggerFlags == 0)
						item->ItemFlags[3] = i;
				}
			}
		}
	}

	void FlameEmitter3Control(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			if (item->TriggerFlags)
			{
				SoundEffect(SFX_TR4_ELEC_ARCING_LOOP, &item->Position, 0);

				byte g = (GetRandomControl() & 0x3F) + 192;
				byte b = (GetRandomControl() & 0x3F) + 192;

				PHD_VECTOR src;
				PHD_VECTOR dest;

				src.x = item->Position.xPos;
				src.y = item->Position.yPos;
				src.z = item->Position.zPos;

				if (!(GlobalCounter & 3))
				{
					if (item->TriggerFlags == 2 || item->TriggerFlags == 4)
					{
						dest.x = item->Position.xPos + 2048 * phd_sin(item->Position.yRot + ANGLE(180));
						dest.y = item->Position.yPos;
						dest.z = item->Position.zPos + 2048 * phd_cos(item->Position.yRot + ANGLE(180));

						if (GetRandomControl() & 3)
						{
							TriggerLightning(
								&src, 
								&dest, 
								(GetRandomControl() & 0x1F) + 64, 
								0, 
								g, 
								b, 
								24, 
								0, 
								32, 
								3);
						}
						else
						{
							TriggerLightning(
								&src, 
								&dest, 
								(GetRandomControl() & 0x1F) + 96,
								0,
								g, 
								b,
								32,
								LI_SPLINE,
								32, 
								3);
						}
					}
				}

				if (item->TriggerFlags >= 3 && !(GlobalCounter & 1))
				{
					short targetItemNumber = item->ItemFlags[((GlobalCounter >> 2) & 1) + 2];
					ITEM_INFO* targetItem = &g_Level.Items[targetItemNumber];

					dest.x = 0;
					dest.y = -64;
					dest.z = 20;
					GetJointAbsPosition(targetItem, &dest, 0);

					if (!(GlobalCounter & 3))
					{
						if (GetRandomControl() & 3)
						{
							TriggerLightning(
								&src, 
								&dest,
								(GetRandomControl() & 0x1F) + 64,
								0,
								g,
								b,
								24, 
								0, 
								32,
								5);
						}
						else
						{
							TriggerLightning(
								&src,
								&dest,
								(GetRandomControl() & 0x1F) + 96,
								0,
								g,
								b,
								32,
								LI_SPLINE,
								32,
								5);
						}
					}
					if (item->TriggerFlags != 3 || targetItem->TriggerFlags)
						TriggerLightningGlow(dest.x, dest.y, dest.z, 64, 0, g, b);
				}

				if ((GlobalCounter & 3) == 2)
				{
					src.x = item->Position.xPos;
					src.y = item->Position.yPos;
					src.z = item->Position.zPos;

					dest.x = (GetRandomControl() & 0x1FF) + src.x - 256;
					dest.y = (GetRandomControl() & 0x1FF) + src.y - 256;
					dest.z = (GetRandomControl() & 0x1FF) + src.z - 256;

					TriggerLightning(
						&src, 
						&dest, 
						(GetRandomControl() & 0xF) + 16,
						0,
						g,
						b,
						24, 
						LI_SPLINE | LI_MOVEEND,
						32,
						3);
					TriggerLightningGlow(dest.x, dest.y, dest.z, 64, 0, g, b);
				}
			}
			else
			{
				// Small fires
				if (item->ItemFlags[0] != 0)
				{
					item->ItemFlags[0]--;
				}
				else
				{
					item->ItemFlags[0] = (GetRandomControl() & 3) + 8;
					short random = GetRandomControl() & 0x3F;
					if (item->ItemFlags[1] == random)
						random = (random + 13) & 0x3F;
					item->ItemFlags[1] = random;
				}

				int x, z, i;

				if (!(Wibble & 4))
				{
					i = item->ItemFlags[1] & 7;
					x = 16 * (Flame3xzoffs[i][0] - 32);
					z = 16 * (Flame3xzoffs[i][1] - 32);
					TriggerFireFlame(x + item->Position.xPos, item->Position.yPos, z + item->Position.zPos, -1, 2);
				}
				else
				{
					i = item->ItemFlags[1] >> 3;
					x = 16 * (Flame3xzoffs[i + 8][0] - 32);
					z = 16 * (Flame3xzoffs[i + 8][1] - 32);
					TriggerFireFlame(x + item->Position.xPos, item->Position.yPos, z + item->Position.zPos, -1, 2);
				}

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Position, 0);

				TriggerDynamicLight(x, item->Position.yPos, z, 12, (GetRandomControl() & 0x3F) + 192, ((GetRandomControl() >> 4) & 0x1F) + 96, 0);

				PHD_3DPOS pos;
				pos.xPos = item->Position.xPos;
				pos.yPos = item->Position.yPos;
				pos.zPos = item->Position.zPos;

				if (ItemNearLara(&pos, 600))
				{
					if ((!Lara.burn) && Lara.Control.WaterStatus != WaterStatus::FlyCheat)
					{
						LaraItem->HitPoints -= 5;
						LaraItem->HitStatus = true;

						int dx = LaraItem->Position.xPos - item->Position.xPos;
						int dz = LaraItem->Position.zPos - item->Position.zPos;

						if (SQUARE(dx) + SQUARE(dz) < SQUARE(450))
							LaraBurn(LaraItem);
					}
				}
			}
		}
	}

	void FlameEmitterCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (Lara.Control.WeaponControl.GunType != WEAPON_TORCH
			|| Lara.Control.HandStatus != HandStatus::WeaponReady
			|| Lara.LeftArm.Locked
			|| Lara.LitTorch == (item->Status & 1)
			|| item->Timer == -1
			|| !(TrInput & IN_ACTION)
			|| l->ActiveState != LS_IDLE
			|| l->AnimNumber != LA_STAND_IDLE
			|| l->Airborne)
		{
			if (item->ObjectNumber == ID_BURNING_ROOTS)
				ObjectCollision(itemNumber, l, coll);
		}
		else
		{
			switch (item->ObjectNumber)
			{
			case ID_FLAME_EMITTER:
				FireBounds.boundingBox.X1 = -256;
				FireBounds.boundingBox.X2 = 256;
				FireBounds.boundingBox.Y1 = 0;
				FireBounds.boundingBox.Y2 = 1024;
				FireBounds.boundingBox.Z1 = -800;
				FireBounds.boundingBox.Z2 = 800;
				break;

			case ID_FLAME_EMITTER2:
				FireBounds.boundingBox.X1 = -256;
				FireBounds.boundingBox.X2 = 256;
				FireBounds.boundingBox.Y1 = 0;
				FireBounds.boundingBox.Y2 = 1024;
				FireBounds.boundingBox.Z1 = -600;
				FireBounds.boundingBox.Z2 = 600;
				break;

			case ID_BURNING_ROOTS:
				FireBounds.boundingBox.X1 = -384;
				FireBounds.boundingBox.X2 = 384;
				FireBounds.boundingBox.Y1 = 0;
				FireBounds.boundingBox.Y2 = 2048;
				FireBounds.boundingBox.Z1 = -384;
				FireBounds.boundingBox.Z2 = 384;
				break;

			}

			short oldYrot = item->Position.yRot;
			item->Position.yRot = l->Position.yRot;

			if (TestLaraPosition(&FireBounds, item, l))
			{
				if (item->ObjectNumber == ID_BURNING_ROOTS)
				{
					l->AnimNumber = LA_TORCH_LIGHT_5;
				}
				else
				{
					int dy = abs(l->Position.yPos - item->Position.yPos);
					l->ItemFlags[3] = 1;
					l->AnimNumber = (dy >> 8) + LA_TORCH_LIGHT_1;
				}

				l->ActiveState = LS_MISC_CONTROL;
				l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
				Lara.Flare.ControlLeft = false;
				Lara.LeftArm.Locked = true;
				Lara.InteractedItem = itemNumber;
			}

			item->Position.yRot = oldYrot;
		}

		if (Lara.InteractedItem == itemNumber
			&& item->Status != ITEM_ACTIVE
			&& l->ActiveState == LS_MISC_CONTROL)
		{
			if (l->AnimNumber >= LA_TORCH_LIGHT_1 && l->AnimNumber <= LA_TORCH_LIGHT_5)
			{
				if (l->FrameNumber - g_Level.Anims[l->AnimNumber].frameBase == 40)
				{
					TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);

					item->Flags |= 0x3E00;
					item->ItemFlags[3] = 0;
					item->Status = ITEM_ACTIVE;

					AddActiveItem(itemNumber);
				}
			}
		}
	}
}