#include "framework.h"
#include "tr4_slicerdicer.h"
#include "Specific/level.h"
#include "Sound/sound.h"
#include "Game/items.h"
#include "Game/animation.h"
#include "Specific/trmath.h"

namespace TEN::Entities::TR4
{
	void InitialiseSlicerDicer(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		int dx = phd_sin(item->pos.yRot + ANGLE(90.0f)) * 512;
		int dz = phd_cos(item->pos.yRot + ANGLE(90.0f)) * 512;

		item->pos.xPos += dx;
		item->pos.zPos += dz;

		item->itemFlags[0] = item->pos.xPos / 256;
		item->itemFlags[1] = (item->pos.yPos - 4608) / 256;
		item->itemFlags[2] = item->pos.zPos / 256;
		item->itemFlags[3] = 50;
	}

	void SlicerDicerControl(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP1, &item->pos, 0);
		SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP2, &item->pos, 0);

		item->pos.xPos = (item->itemFlags[0] * 256) + 4608 * phd_cos(item->triggerFlags) * phd_sin(item->pos.yRot);
		item->pos.yPos = (item->itemFlags[1] * 256) - 4608 * phd_sin(item->triggerFlags);
		item->pos.zPos = (item->itemFlags[2] * 256) + 4608 * phd_cos(item->triggerFlags) * phd_cos(item->pos.yRot);

		item->triggerFlags += 170;

		short roomNumber = item->roomNumber;
		GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		if (item->roomNumber != roomNumber)
			ItemNewRoom(itemNum, roomNumber);

		AnimateItem(item);
	}
}