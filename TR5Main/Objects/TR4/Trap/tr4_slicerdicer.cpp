#include "framework.h"
#include "tr4_slicerdicer.h"
#include "Specific/level.h"
#include "Sound/sound.h"
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/animation.h"
#include "Specific/trmath.h"

namespace TEN::Entities::TR4
{
	void InitialiseSlicerDicer(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		int dx = phd_sin(item->Position.yRot + ANGLE(90.0f)) * 512;
		int dz = phd_cos(item->Position.yRot + ANGLE(90.0f)) * 512;

		item->Position.xPos += dx;
		item->Position.zPos += dz;

		item->ItemFlags[0] = item->Position.xPos / 256;
		item->ItemFlags[1] = (item->Position.yPos - 4608) / 256;
		item->ItemFlags[2] = item->Position.zPos / 256;
		item->ItemFlags[3] = 50;
	}

	void SlicerDicerControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP1, &item->Position, 0);
		SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP2, &item->Position, 0);

		item->Position.xPos = (item->ItemFlags[0] * 256) + 4608 * phd_cos(item->TriggerFlags) * phd_sin(item->Position.yRot);
		item->Position.yPos = (item->ItemFlags[1] * 256) - 4608 * phd_sin(item->TriggerFlags);
		item->Position.zPos = (item->ItemFlags[2] * 256) + 4608 * phd_cos(item->TriggerFlags) * phd_cos(item->Position.yRot);

		item->TriggerFlags += 170;

		auto probedRoomNumber = GetCollision(item).RoomNumber;
		if (item->RoomNumber != probedRoomNumber)
			ItemNewRoom(itemNumber, probedRoomNumber);

		AnimateItem(item);
	}
}
