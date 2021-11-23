#include "framework.h"
#include "flame_emitters.h"
#include "Sound/sound.h"
#include "Specific/trmath.h"
#include "sphere.h"
#include "level.h"
#include "lara.h"
#include "effects/tomb4fx.h"
#include "collide.h"
#include "animation.h"
#include "Game/effects/effects.h"
#include "weather.h"
#include "Specific/setup.h"
#include "Game/effects/lightning.h"
#include "Game/effects/lara_fx.h"
#include "items.h"

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
			if (item->triggerFlags < 0)
			{
				short flags = -item->triggerFlags;
				if ((flags & 7) == 2 || (flags & 7) == 7)
				{
					SoundEffect(SFX_TR4_FLAME_EMITTER, &item->pos, 0);
					TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
					TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
						(GetRandomControl() & 3) + 20,
						(GetRandomControl() & 0x3F) + 192,
						(GetRandomControl() & 0x1F) + 96, 0);
				}
				else
				{
					if (item->itemFlags[0])
					{
						if (item->itemFlags[1])
							item->itemFlags[1] = item->itemFlags[1] - (item->itemFlags[1] >> 2);

						if (item->itemFlags[2] < 256)
							item->itemFlags[2] += 8;

						item->itemFlags[0]--;
						if (item->itemFlags[0] == 1)
							item->itemFlags[3] = (GetRandomControl() & 0x3F) + 150;
					}
					else
					{
						if (!--item->itemFlags[3])
						{
							if (flags >> 3)
								item->itemFlags[0] = (GetRandomControl() & 0x1F) + 30 * (flags >> 3);
							else
								item->itemFlags[0] = (GetRandomControl() & 0x3F) + 60;
						}

						if (item->itemFlags[2])
							item->itemFlags[2] -= 8;

						if (item->itemFlags[1] > -8192)
							item->itemFlags[1] -= 512;
					}

					if (item->itemFlags[2])
						AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, SP_NORMALFIRE, item->roomNumber, item->itemFlags[2]);

					if (item->itemFlags[1])
					{
						SoundEffect(SFX_TR4_FLAME_EMITTER, &item->pos, 0);

						if (item->itemFlags[1] <= -8192)
							TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
						else
							TriggerSuperJetFlame(item, item->itemFlags[1], GlobalCounter & 1);

						TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
							(-item->itemFlags[1] >> 10) - (GetRandomControl() & 1) + 16,
							(GetRandomControl() & 0x3F) + 192,
							(GetRandomControl() & 0x1F) + 96, 0);
					}
					else
					{
						r = (GetRandomControl() & 0x3F) + 192;
						g = (GetRandomControl() & 0x1F) + 96;
						falloff = 10 - (GetRandomControl() & 1);
						TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
							10 - (GetRandomControl() & 1),
							(GetRandomControl() & 0x3F) + 192,
							(GetRandomControl() & 0x1F) + 96, 0);
					}
				}

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
			}
			else
			{
				if (item->triggerFlags < 8)
					FlameEmitterFlags[item->triggerFlags] = true;

				AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, SP_BIGFIRE, item->roomNumber, 0);

				TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
					16 - (GetRandomControl() & 1),
					(GetRandomControl() & 0x3F) + 192,
					(GetRandomControl() & 0x1F) + 96, 0);

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, 0);

				if (!Lara.burn
					&& ItemNearLara(&item->pos, 600)
					&& (SQUARE(LaraItem->pos.xPos - item->pos.xPos) +
						SQUARE(LaraItem->pos.zPos - item->pos.zPos) < SQUARE(512))
					&& Lara.waterStatus != LW_FLYCHEAT)
				{
					LaraBurn(LaraItem);
				}
			}
		}
		else
		{
			if (item->triggerFlags > 0 && item->triggerFlags < 8)
				FlameEmitterFlags[item->triggerFlags] = false;
		}
	}

	void FlameEmitter2Control(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			// If not an emitter for flipmaps
			if (item->triggerFlags >= 0)
			{
				// If not a moving flame
				if (item->triggerFlags != 2)
				{
					if (item->triggerFlags == 123)
					{
						// Middle of the block
						AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, SP_SMALLFIRE, item->roomNumber, item->itemFlags[3]);
					}
					else
					{
						AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, SP_SMALLFIRE - item->triggerFlags, item->roomNumber, item->itemFlags[3]);
					}
				}

				if (item->triggerFlags == 0 || item->triggerFlags == 2)
				{
					if (item->itemFlags[3])
					{
						TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
							10,
							((GetRandomControl() & 0x3F) + 192) * item->itemFlags[3] >> 8,
							(GetRandomControl() & 0x1F) + 96 * item->itemFlags[3] >> 8,
							0);
					}
					else
					{
						TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
							10,
							(GetRandomControl() & 0x3F) + 192,
							(GetRandomControl() & 0x1F) + 96,
							0);
					}
				}

				if (item->triggerFlags == 2)
				{
					item->pos.xPos += phd_sin(item->pos.yRot - ANGLE(180));
					item->pos.zPos += phd_cos(item->pos.yRot - ANGLE(180));

					short roomNumber = item->roomNumber;
					FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

					if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER)
					{
						Weather.Flash(255, 128, 0, 0.03f);
						KillItem(itemNumber);
						return;
					}

					if (item->roomNumber != roomNumber)
					{
						ItemNewRoom(itemNumber, roomNumber);
					}

					item->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);;

					if (Wibble & 7)
					{
						TriggerFireFlame(item->pos.xPos, item->pos.yPos - 32, item->pos.zPos, -1, 1);
					}
				}

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
			}
			else if (item->itemFlags[0] == 0)
			{
				DoFlipMap(-item->triggerFlags);
				FlipMap[-item->triggerFlags] ^= 0x3E00u;
				item->itemFlags[0] = 1;
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
				if (Lara.burnCount)
				{
					Lara.burnCount--;
					if (!Lara.burnCount)
						Lara.burnSmoke = true;
				}

				TriggerFireFlame(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, -1, 255 - Lara.burnSmoke);
			}
		}

		byte r = (GetRandomControl() & 0x3F) + 192;
		byte g = (GetRandomControl() & 0x1F) + 96;
		byte b;

		PHD_VECTOR pos{ 0,0,0 };
		GetLaraJointPosition(&pos, LM_HIPS);

		if (!Lara.burnSmoke)
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

		if (LaraItem->roomNumber != fx->roomNumber)
			EffectNewRoom(fxNumber, LaraItem->roomNumber);

		int wh = GetWaterHeight(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, fx->roomNumber);
		if (wh == NO_HEIGHT || fx->pos.yPos <= wh || Lara.burnBlue)
		{
			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &fx->pos, 0);

			LaraItem->hitPoints -= 7;
			LaraItem->hitStatus = true;
		}
		else
		{
			KillEffect(fxNumber);
			Lara.burn = false;
		}

		if (Lara.waterStatus == LW_FLYCHEAT)
		{
			KillEffect(fxNumber);
			Lara.burn = false;
		}
	}

	void InitialiseFlameEmitter(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->triggerFlags < 0)
		{
			item->itemFlags[0] = (GetRandomControl() & 0x3F) + 90;
			item->itemFlags[2] = 256;

			if (((-item->triggerFlags) & 7) == 7)
			{
				switch (item->pos.yRot)
				{
				case 0:
					item->pos.zPos += 512;
					break;

				case 0x4000:
					item->pos.xPos += 512;
					break;

				case -0x8000:
					item->pos.zPos -= 512;
					break;

				case -0x4000:
					item->pos.xPos -= 512;
					break;
				}
			}
		}
	}

	void InitialiseFlameEmitter2(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		item->pos.yPos -= 64;

		if (item->triggerFlags != 123)
		{
			switch (item->pos.yRot)
			{
			case 0:
				if (item->triggerFlags == 2)
					item->pos.zPos += 80;
				else
					item->pos.zPos += 256;
				break;

			case 0x4000:
				if (item->triggerFlags == 2)
					item->pos.xPos += 80;
				else
					item->pos.xPos += 256;
				break;

			case -0x8000:
				if (item->triggerFlags == 2)
					item->pos.zPos -= 80;
				else
					item->pos.zPos -= 256;
				break;

			case -0x4000:
				if (item->triggerFlags == 2)
					item->pos.xPos -= 80;
				else
					item->pos.xPos -= 256;
				break;
			}
		}
	}

	void InitialiseFlameEmitter3(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->triggerFlags >= 3)
		{
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				ITEM_INFO* currentItem = &g_Level.Items[i];

				if (currentItem->objectNumber == ID_ANIMATING3)
				{
					if (currentItem->triggerFlags == item->triggerFlags)
						item->itemFlags[2] = i;
					else if (currentItem->triggerFlags == 0)
						item->itemFlags[3] = i;
				}
			}
		}
	}

	void FlameEmitter3Control(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			if (item->triggerFlags)
			{
				SoundEffect(SFX_TR4_ELEC_ARCING_LOOP, &item->pos, 0);

				byte g = (GetRandomControl() & 0x3F) + 192;
				byte b = (GetRandomControl() & 0x3F) + 192;

				PHD_VECTOR src;
				PHD_VECTOR dest;

				src.x = item->pos.xPos;
				src.y = item->pos.yPos;
				src.z = item->pos.zPos;

				if (!(GlobalCounter & 3))
				{
					if (item->triggerFlags == 2 || item->triggerFlags == 4)
					{
						dest.x = item->pos.xPos + 2048 * phd_sin(item->pos.yRot + ANGLE(180));
						dest.y = item->pos.yPos;
						dest.z = item->pos.zPos + 2048 * phd_cos(item->pos.yRot + ANGLE(180));

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

				if (item->triggerFlags >= 3 && !(GlobalCounter & 1))
				{
					short targetItemNumber = item->itemFlags[((GlobalCounter >> 2) & 1) + 2];
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
					if (item->triggerFlags != 3 || targetItem->triggerFlags)
						TriggerLightningGlow(dest.x, dest.y, dest.z, 64, 0, g, b);
				}

				if ((GlobalCounter & 3) == 2)
				{
					src.x = item->pos.xPos;
					src.y = item->pos.yPos;
					src.z = item->pos.zPos;

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
				if (item->itemFlags[0] != 0)
				{
					item->itemFlags[0]--;
				}
				else
				{
					item->itemFlags[0] = (GetRandomControl() & 3) + 8;
					short random = GetRandomControl() & 0x3F;
					if (item->itemFlags[1] == random)
						random = (random + 13) & 0x3F;
					item->itemFlags[1] = random;
				}

				int x, z, i;

				if (!(Wibble & 4))
				{
					i = item->itemFlags[1] & 7;
					x = 16 * (Flame3xzoffs[i][0] - 32);
					z = 16 * (Flame3xzoffs[i][1] - 32);
					TriggerFireFlame(x + item->pos.xPos, item->pos.yPos, z + item->pos.zPos, -1, 2);
				}
				else
				{
					i = item->itemFlags[1] >> 3;
					x = 16 * (Flame3xzoffs[i + 8][0] - 32);
					z = 16 * (Flame3xzoffs[i + 8][1] - 32);
					TriggerFireFlame(x + item->pos.xPos, item->pos.yPos, z + item->pos.zPos, -1, 2);
				}

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, 0);

				TriggerDynamicLight(x, item->pos.yPos, z, 12, (GetRandomControl() & 0x3F) + 192, ((GetRandomControl() >> 4) & 0x1F) + 96, 0);

				PHD_3DPOS pos;
				pos.xPos = item->pos.xPos;
				pos.yPos = item->pos.yPos;
				pos.zPos = item->pos.zPos;

				if (ItemNearLara(&pos, 600))
				{
					if ((!Lara.burn) && Lara.waterStatus != LW_FLYCHEAT)
					{
						LaraItem->hitPoints -= 5;
						LaraItem->hitStatus = true;

						int dx = LaraItem->pos.xPos - item->pos.xPos;
						int dz = LaraItem->pos.zPos - item->pos.zPos;

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

		if (Lara.gunType != WEAPON_TORCH
			|| Lara.gunStatus != LG_READY
			|| Lara.leftArm.lock
			|| Lara.litTorch == (item->status & 1)
			|| item->timer == -1
			|| !(TrInput & IN_ACTION)
			|| l->currentAnimState != LS_STOP
			|| l->animNumber != LA_STAND_IDLE
			|| l->gravityStatus)
		{
			if (item->objectNumber == ID_BURNING_ROOTS)
				ObjectCollision(itemNumber, l, coll);
		}
		else
		{
			switch (item->objectNumber)
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

			short oldYrot = item->pos.yRot;
			item->pos.yRot = l->pos.yRot;

			if (TestLaraPosition(&FireBounds, item, l))
			{
				if (item->objectNumber == ID_BURNING_ROOTS)
				{
					l->animNumber = LA_TORCH_LIGHT_5;
				}
				else
				{
					int dy = abs(l->pos.yPos - item->pos.yPos);
					l->itemFlags[3] = 1;
					l->animNumber = (dy >> 8) + LA_TORCH_LIGHT_1;
				}

				l->currentAnimState = LS_MISC_CONTROL;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				Lara.flareControlLeft = false;
				Lara.leftArm.lock = 3;
				Lara.interactedItem = itemNumber;
			}

			item->pos.yRot = oldYrot;
		}

		if (Lara.interactedItem == itemNumber
			&& item->status != ITEM_ACTIVE
			&& l->currentAnimState == LS_MISC_CONTROL)
		{
			if (l->animNumber >= LA_TORCH_LIGHT_1 && l->animNumber <= LA_TORCH_LIGHT_5)
			{
				if (l->frameNumber - g_Level.Anims[l->animNumber].frameBase == 40)
				{
					TestTriggers(item, true, item->flags & IFLAG_ACTIVATION_MASK);

					item->flags |= 0x3E00;
					item->itemFlags[3] = 0;
					item->status = ITEM_ACTIVE;

					AddActiveItem(itemNumber);
				}
			}
		}
	}
}