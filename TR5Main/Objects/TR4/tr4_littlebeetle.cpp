#include "../newobjects.h"

void InitialiseLittleBeetle(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->itemFlags[0] = (item->triggerFlags / 1000) & 1;
	item->itemFlags[1] = (item->triggerFlags / 1000) & 2;
	item->itemFlags[2] = (item->triggerFlags / 1000) & 4;

	item->triggerFlags = item->triggerFlags % 1000;

	if (!item->itemFlags[1])
	{
		if (item->pos.yRot <= 4096 || item->pos.yRot >= 28672)
		{
			if (!(item->pos.yRot >= -4096 || item->pos.yRot <= -28672))
				item->pos.xPos += 512;
		}
		else
		{
			item->pos.xPos -= 512;
		}

		if (item->pos.yRot <= -8192 || item->pos.yRot >= 0x2000)
		{
			if (item->pos.yRot < -20480 || item->pos.yRot > 20480)
			{
				item->pos.zPos += 512;
			}
		}
		else
		{
			item->pos.zPos -= 512;
		}
	}
}

void LittleBeetleControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
}