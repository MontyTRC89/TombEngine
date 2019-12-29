#include "..\newobjects.h"

short GetNextRat()
{
	short ratNum = NextRat;
	int i = 0;
	RAT_STRUCT* rat = &Rats[NextRat];

	while (rat->on)
	{
		if (ratNum == 31)
		{
			rat = &Rats[0];
			ratNum = 0;
		}
		else
		{
			ratNum++;
			rat++;
		}

		i++;

		if (i >= 32)
			return NO_ITEM;
	}

	NextRat = (ratNum + 1) & 0x1F;
	return ratNum;
}

void ControlLittleRats(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (item->triggerFlags)
	{
		if (!item->itemFlags[2] || !(GetRandomControl() & 0xF))
		{
			item->triggerFlags--;

			if (item->itemFlags[2] && GetRandomControl() & 1)
				item->itemFlags[2]--;

			short ratNum = GetNextRat();
			if (ratNum != -1)
			{
				RAT_STRUCT* rat = &Rats[ratNum];

				rat->pos.xPos = item->pos.xPos;
				rat->pos.yPos = item->pos.yPos;
				rat->pos.zPos = item->pos.zPos;
				rat->roomNumber = item->roomNumber;

				if (item->itemFlags[0])
				{
					rat->pos.yRot = 2 * GetRandomControl();
					rat->fallspeed = -16 - (GetRandomControl() & 0x1F);
				}
				else
				{
					rat->fallspeed = 0;
					rat->pos.yRot = item->pos.yRot + (GetRandomControl() & 0x3FFF) - ANGLE(45);
				}

				rat->pos.xRot = 0;
				rat->pos.zRot = 0;
				rat->on = 1;
				rat->flags = GetRandomControl() & 0x1E;
				rat->speed = (GetRandomControl() & 0x1F) + 1;
			}
		}
	}
}