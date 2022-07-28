#include "framework.h"
#include "tr5_light.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/effects/effects.h"
#include "Sound/sound.h"
#include "Specific/trmath.h"
#include "Game/animation.h"
#include "Game/items.h"

void PulseLightControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		item->ItemFlags[0] -= 1024;

		long pulse = 256 * phd_sin(item->ItemFlags[0] + 4 * (item->Pose.Position.y & 0x3FFF));
		pulse = abs(pulse);
		if (pulse > 255)
			pulse = 255;

		TriggerDynamicLight(
			item->Pose.Position.x,
			item->Pose.Position.y,
			item->Pose.Position.z,
			24,
			(pulse * 8 * (item->TriggerFlags & 0x1F)) / 512,
			(pulse * ((item->TriggerFlags / 4) & 0xF8)) / 512,
			(pulse * ((item->TriggerFlags / 128) & 0xF8)) / 512);
	}
}

void TriggerAlertLight(int x, int y, int z, int r, int g, int b, int angle, short room, int falloff)
{
	GameVector start;
	start.x = x;
	start.y = y;
	start.z = z;
	GetFloor(x, y, z, &room);
	start.roomNumber = room;

	GameVector end;
	end.x = x + 16384 * phd_sin(16 * angle);
	end.y = y;
	end.z = z + 16384 * phd_cos(16 * angle);

	if (!LOS(&start, &end))
		TriggerDynamicLight(end.x, end.y, end.z, falloff, r, g, b);
}

void StrobeLightControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		item->Pose.Orientation.y += ANGLE(16.0f);

		byte r = 8 * (item->TriggerFlags & 0x1F);
		byte g = (item->TriggerFlags / 4) & 0xF8;
		byte b = (item->TriggerFlags / 128) & 0xF8;

		TriggerAlertLight(
			item->Pose.Position.x,
			item->Pose.Position.y - 512,
			item->Pose.Position.z,
			r, g, b,
			((item->Pose.Orientation.y + 22528) / 16) & 0xFFF,
			item->RoomNumber,
			12);

		TriggerDynamicLight(
			item->Pose.Position.x + 256 * phd_sin(item->Pose.Orientation.y + 22528),
			item->Pose.Position.y - 768,
			item->Pose.Position.z + 256 * phd_cos(item->Pose.Orientation.y + 22528),
			8,
			r, g, b);
	}
}

void ColorLightControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		TriggerDynamicLight(
			item->Pose.Position.x,
			item->Pose.Position.y,
			item->Pose.Position.z,
			24,
			8 * (item->TriggerFlags & 0x1F),
			(item->TriggerFlags / 4) & 0xF8,
			(item->TriggerFlags / 128) & 0xF8);
	}
}

void ElectricalLightControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
	{
		item->ItemFlags[0] = 0;
		return;
	}

	int intensity = 0;

	if (item->TriggerFlags > 0)
	{
		if (item->ItemFlags[0] < 16)
		{
			intensity = 4 * (GetRandomControl() & 0x3F);
			item->ItemFlags[0]++;
		}
		else if (item->ItemFlags[0] >= 96)
		{
			if (item->ItemFlags[0] >= 160)
				intensity = 255 - (GetRandomControl() & 0x1F);
			else
			{
				intensity = 96 - (GetRandomControl() & 0x1F);
				if (!(GetRandomControl() & 0x1F) && item->ItemFlags[0] > 128)
					item->ItemFlags[0] = 160;
				else
					item->ItemFlags[0]++;
			}
		}
		else
		{
			if (Wibble & 0x3F && GetRandomControl() & 7)
			{
				intensity = GetRandomControl() & 0x3F;
				item->ItemFlags[0]++;
			}
			else
			{
				intensity = 192 - (GetRandomControl() & 0x3F);
				item->ItemFlags[0]++;
			}
		}
	}
	else
	{
		if (item->ItemFlags[0] <= 0)
		{
			item->ItemFlags[0] = (GetRandomControl() & 3) + 4;
			item->ItemFlags[1] = (GetRandomControl() & 0x7F) + 128;
			item->ItemFlags[2] = GetRandomControl() & 1;
		}

		item->ItemFlags[0]--;

		if (!item->ItemFlags[2])
		{
			item->ItemFlags[0]--;

			intensity = item->ItemFlags[1] - (GetRandomControl() & 0x7F);
			if (intensity > 64)
				SoundEffect(SFX_TR5_ELECTRIC_LIGHT_CRACKLES, &item->Pose, SoundEnvironment::Land, 1.0f, (float)intensity / 192.0f);
		}
		else
			return;
	}

	TriggerDynamicLight(
		item->Pose.Position.x,
		item->Pose.Position.y,
		item->Pose.Position.z,
		24,
		(intensity * 8 * (item->TriggerFlags & 0x1F)) / 256,
		(intensity * ((item->TriggerFlags / 4) & 0xF8)) / 256,
		(intensity * ((item->TriggerFlags / 128) & 0xF8)) / 256);
}

void BlinkingLightControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		item->ItemFlags[0]--;

		if (item->ItemFlags[0] >= 3)
			item->MeshBits = 1;
		else
		{
			Vector3Int pos = { 0, 0, 0 };
			GetJointAbsPosition(item, &pos, 0);

			TriggerDynamicLight(
				pos.x,
				pos.y,
				pos.z,
				16,
				8 * (item->TriggerFlags & 0x1F),
				(item->TriggerFlags / 4) & 0xF8,
				(item->TriggerFlags / 128) & 0xF8);

			item->MeshBits = 2;

			if (item->ItemFlags[0] < 0)
				item->ItemFlags[0] = 30;
		}
	}
}
