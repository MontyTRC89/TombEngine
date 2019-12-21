#include "traps.h"
#include "..\Global\global.h"
#include "items.h"
#include "effect2.h"
#include "tomb4fx.h"
#include "effects.h"
#include "lara.h"

void LaraBurn()
{
	if (!Lara.burn && !Lara.burnSmoke)
	{
		short fxNum = CreateNewEffect(LaraItem->roomNumber);
		if (fxNum != NO_ITEM)
		{
			Effects[fxNum].objectNumber = ID_FLAME;
			Lara.burn = true;
		}
	}
}

void FlameEmitterControl(short itemNumber)
{
	byte r, g, b;
	int falloff;

	ITEM_INFO* item = &Items[itemNumber];
	if (TriggerActive(item))
	{
		if (item->triggerFlags < 0)
		{
			short flags = -item->triggerFlags;
			if ((flags & 7) == 2 || (flags & 7) == 7)
			{
				PHD_3DPOS* pos = &item->pos;
				SoundEffect(SFX_D_METAL_CAGE_OPEN, &item->pos, 0);
				TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
				g = (GetRandomControl() & 0x1F) + 96;
				r = (GetRandomControl() & 0x3F) + 192;
				falloff = (GetRandomControl() & 3) + 20;
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
					if (!item->itemFlags[0])
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
					AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, 0, item->roomNumber, item->itemFlags[2]);

				if (item->itemFlags[1])
				{
					SoundEffect(SFX_D_METAL_CAGE_OPEN, &item->pos, 0);
					
					if (item->itemFlags[1] <= -8192)
						TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
					else
						TriggerSuperJetFlame(item, item->itemFlags[1], GlobalCounter & 1);
					
					r = (GetRandomControl() & 0x3F) + 192;
					g = (GetRandomControl() & 0x1F) + 96;
					falloff = (-item->itemFlags[1] >> 10) - (GetRandomControl() & 1) + 16;
					TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, falloff, r, g, 0);
				}
				else
				{
					r = (GetRandomControl() & 0x3F) + 192;
					g = (GetRandomControl() & 0x1F) + 96;
					falloff = 10 - (GetRandomControl() & 1);
					TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, falloff, r, g, 0);
				}
			}

			SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
			return;
		}

		AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, 2, item->roomNumber, 0);
		
		r = (GetRandomControl() & 0x3F) + 192;
		g = (GetRandomControl() & 0x1F) + 96;
		falloff = 16 - (GetRandomControl() & 1);
		TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, falloff, r, g, 0);
		
		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
		
		if (!Lara.burn
			&& item->triggerFlags != 33
			&& ItemNearLara(&item->pos, 600)
			&& (SQUARE(LaraItem->pos.xPos - item->pos.xPos) + SQUARE(LaraItem->pos.zPos - item->pos.zPos) < 0x40000))
		{
			LaraBurn();
		}
	}
}

void FlameEmitter2Control(short itemNumber)//5A1BC, 5A638 (F)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->triggerFlags < 0)
		{
			if (item->itemFlags[0])
			{
				if (item->itemFlags[0] == 1)
				{
					DoFlipMap(-item->triggerFlags);
					FlipMap[-item->triggerFlags] ^= 0x3E00u;
					item->itemFlags[0] = 2;
				}
			}
			else
			{
				if (item->triggerFlags < -100)
					item->triggerFlags = item->triggerFlags + 100;

				item->itemFlags[0] = 1;
			}
		}
		else
		{
			if (item->triggerFlags != 2)
			{
				if (item->triggerFlags == 123)
					AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, 1, item->roomNumber, item->itemFlags[3]);
				else
					AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, 1 - item->triggerFlags, item->roomNumber, item->itemFlags[3]);
			}

			if (item->triggerFlags == 0 || item->triggerFlags == 2)
			{
				int r = (GetRandomControl() & 0x3F) + 192;
				int g = (GetRandomControl() & 0x1F) + 96;

				if (item->itemFlags[3])
				{
					r = r * item->itemFlags[3] >> 8;
					g = g * item->itemFlags[3] >> 8;
				}

				TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, 10, r, g, 0);
			}

			SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
		}
	}
}

void FlameControl(short fxNumber)
{
	FX_INFO* fx = &Effects[fxNumber];

	for (int i = 0; i < 14; i++)
	{
		if (!(Wibble & 0xC))
		{
			fx->pos.xPos = 0;
			fx->pos.yPos = 0;
			fx->pos.zPos = 0;

			GetLaraJointPosition((PHD_VECTOR*)& fx->pos, i);

			if (Lara.BurnCount)
			{
				Lara.BurnCount--;
				if (!Lara.BurnCount)
					Lara.burnSmoke = true;
			}

			TriggerFireFlame(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, -1, 255 - ((Lara.currentZvel >> 10) & 1));
		}
	}

	byte r = (GetRandomControl() & 0x3F) + 192;
	byte g = (GetRandomControl() & 0x1F) + 96;
	byte b;

	if (!Lara.burnSmoke)
	{
		if (!Lara.burnBlue)
		{
			TriggerDynamicLight(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 13, r, g, 0);
		}
		else
		{
			if (Lara.burnBlue == 128)
			{
				b = r;
				TriggerDynamicLight(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 13, 0, g, b);
			}
			else if (Lara.burnBlue == 256)
			{
				b = g;
				g = r;
				TriggerDynamicLight(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 13, 0, g, b);
			}
		}
	}

	if (LaraItem->roomNumber != fx->roomNumber)
		EffectNewRoom(fxNumber, LaraItem->roomNumber);
	
	int wh = GetWaterHeight(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, fx->roomNumber);
	if (wh == NO_HEIGHT || fx->pos.yPos <= wh || Lara.burnBlue)
	{
		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &fx->pos, 0);

		LaraItem->hitPoints -= 7;
		LaraItem->hitStatus = true;
	}
	else
	{
		KillEffect(fxNumber);
		Lara.burn = false;
	}
}

void LavaBurn(ITEM_INFO* item)
{
	if (item->hitPoints >= 0 && Lara.waterStatus != LW_FLYCHEAT)
	{
		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, 32000, item->pos.zPos, &roomNumber);
		if (item->floor == GetFloorHeight(floor, item->pos.xPos, 32000, item->pos.zPos))
		{
			item->hitPoints = -1;
			item->hitStatus = true;
			LaraBurn();
		}
	}
}