#include "framework.h"
#include "tr5_light.h"
#include "Specific/level.h"
#include "Game/control/los.h"
#include "Game/effects/effects.h"
#include "Sound/sound.h"
#include "Specific/trmath.h"
#include "Game/animation.h"
#include "Game/items.h"

void PulseLightControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		item->itemFlags[0] -= 1024;

		long pulse = 256 * phd_sin(item->itemFlags[0] + 4 * (item->pos.yPos & 0x3FFF));
		pulse = abs(pulse);
		if (pulse > 255)
			pulse = 255;

		TriggerDynamicLight(
			item->pos.xPos,
			item->pos.yPos,
			item->pos.zPos,
			24,
			(pulse * 8 * (item->triggerFlags & 0x1F)) / 512,
			(pulse * ((item->triggerFlags / 4) & 0xF8)) / 512,
			(pulse * ((item->triggerFlags / 128) & 0xF8)) / 512);
	}
}

void TriggerAlertLight(int x, int y, int z, int r, int g, int b, int angle, short room, int falloff)
{
	GAME_VECTOR source, target;

	source.x = x;
	source.y = y;
	source.z = z;
	GetFloor(x, y, z,&room);
	source.roomNumber = room;
	target.x = x + 16384 * phd_sin(16 * angle);
	target.y = y;
	target.z = z + 16384 * phd_cos(16 * angle);
	if (!LOS(&source,&target))
		TriggerDynamicLight(target.x, target.y, target.z, falloff, r, g, b);
}

void StrobeLightControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		item->pos.yRot += ANGLE(16.0f);

		byte r = 8 * (item->triggerFlags & 0x1F);
		byte g = (item->triggerFlags / 4) & 0xF8;
		byte b = (item->triggerFlags / 128) & 0xF8;

		TriggerAlertLight(
			item->pos.xPos,
			item->pos.yPos - 512,
			item->pos.zPos,
			r, g, b,
			((item->pos.yRot + 22528) / 16) & 0xFFF,
			item->roomNumber,
			12);

		TriggerDynamicLight(
			item->pos.xPos + 256 * phd_sin(item->pos.yRot + 22528),
			item->pos.yPos - 768,
			item->pos.zPos + 256 * phd_cos(item->pos.yRot + 22528),
			8,
			r, g, b);
	}
}

void ColorLightControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		TriggerDynamicLight(
			item->pos.xPos,
			item->pos.yPos,
			item->pos.zPos,
			24,
			8 * (item->triggerFlags & 0x1F),
			(item->triggerFlags / 4) & 0xF8,
			(item->triggerFlags / 128) & 0xF8);
	}
}

void ElectricalLightControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
	{
		item->itemFlags[0] = 0;
		return;
	}

	int intensity = 0;

	if (item->triggerFlags > 0)
	{
		if (item->itemFlags[0] < 16)
		{
			intensity = 4 * (GetRandomControl() & 0x3F);
			item->itemFlags[0]++;
		}
		else if (item->itemFlags[0] >= 96)
		{
			if (item->itemFlags[0] >= 160)
			{
				intensity = 255 - (GetRandomControl() & 0x1F);
			}
			else
			{
				intensity = 96 - (GetRandomControl() & 0x1F);
				if (!(GetRandomControl() & 0x1F) && item->itemFlags[0] > 128)
				{
					item->itemFlags[0] = 160;
				}
				else
				{
					item->itemFlags[0]++;
				}
			}
		}
		else
		{
			if (Wibble & 0x3F && GetRandomControl() & 7)
			{
				intensity = GetRandomControl() & 0x3F;
				item->itemFlags[0]++;
			}
			else
			{
				intensity = 192 - (GetRandomControl() & 0x3F);
				item->itemFlags[0]++;
			}
		}
	}
	else
	{
		if (item->itemFlags[0] <= 0)
		{
			item->itemFlags[0] = (GetRandomControl() & 3) + 4;
			item->itemFlags[1] = (GetRandomControl() & 0x7F) + 128;
			item->itemFlags[2] = GetRandomControl() & 1;
		}

		item->itemFlags[0]--;

		if (!item->itemFlags[2])
		{
			item->itemFlags[0]--;

			intensity = item->itemFlags[1] - (GetRandomControl() & 0x7F);
			if (intensity > 64)
				SoundEffect(1001,&item->pos, 32 * (intensity & 0xFFFFFFF8) | 8);
		}
		else
		{
			return;
		}
	}

	TriggerDynamicLight(
		item->pos.xPos,
		item->pos.yPos,
		item->pos.zPos,
		24,
		(intensity * 8 * (item->triggerFlags & 0x1F)) / 256,
		(intensity * ((item->triggerFlags / 4) & 0xF8)) / 256,
		(intensity * ((item->triggerFlags / 128) & 0xF8)) / 256);
}

void BlinkingLightControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		item->itemFlags[0]--;

		if (item->itemFlags[0] >= 3)
		{
			item->meshBits = 1;
		}
		else
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;
			GetJointAbsPosition(item,&pos, 0);

			TriggerDynamicLight(
				pos.x,
				pos.y,
				pos.z,
				16,
				8 * (item->triggerFlags & 0x1F),
				(item->triggerFlags / 4) & 0xF8,
				(item->triggerFlags / 128) & 0xF8);

			item->meshBits = 2;

			if (item->itemFlags[0] < 0)
				item->itemFlags[0] = 30;
		}
	}
}