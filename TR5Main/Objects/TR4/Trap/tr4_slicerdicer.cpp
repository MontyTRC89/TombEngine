#include "framework.h"
#include "tr4_slicerdicer.h"
#include "level.h"
#include "sound.h"
#include "items.h"
#include "trmath.h"

void InitialiseSlicerDicer(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	int dx = phd_sin(item->pos.yRot + ANGLE(90.0f)) >> 5;
	int dz = phd_cos(item->pos.yRot + ANGLE(90.0f)) >> 5;

	item->pos.xPos += dx;
	item->pos.zPos += dz;

	item->itemFlags[0] = item->pos.xPos >> 8;
	item->itemFlags[1] = (item->pos.yPos - 4608) >> 8;
	item->itemFlags[2] = item->pos.zPos >> 8;
	item->itemFlags[3] = 50;
}

void SlicerDicerControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP1, &item->pos, 0);
	SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP, &item->pos, 0);

	int factor = (9 * phd_cos(item->triggerFlags) << 9 >> W2V_SHIFT) * phd_cos(item->pos.yRot) >> W2V_SHIFT;

	item->pos.xPos = (item->itemFlags[0] << 8) + factor;
	item->pos.yPos = (item->itemFlags[1] << 8) - 4608 * phd_sin(item->triggerFlags);
	item->pos.zPos = (item->itemFlags[2] << 8) + factor;

	item->triggerFlags += 170;

	short roomNumber = item->roomNumber;
	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNum, roomNumber);

	AnimateItem(item);
}