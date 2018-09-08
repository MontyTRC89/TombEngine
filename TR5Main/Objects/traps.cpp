#include "objects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"

void __cdecl FourBladesControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (frameNumber <= 5 || frameNumber >= 58 || frameNumber >= 8 && frameNumber <= 54)
			item->itemFlags[0] = 0;
		else
		{
			if (frameNumber >= 6 && frameNumber <= 7)
			{
				item->itemFlags[3] = 20;
				item->itemFlags[0] = 30;
			}
			else
			{
				if (frameNumber >= 55 && frameNumber <= 57)
				{
					item->itemFlags[3] = 200;
					item->itemFlags[0] = 30;
				}
			}
		}

		AnimateItem(item);
	}
}

void __cdecl BirdBladeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->itemFlags[3] = 100;
	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (frameNumber <= 14 || frameNumber >= 31)
			item->itemFlags[0] = 0;
		else
			item->itemFlags[0] = 6;

		AnimateItem(item);
	}
}

void __cdecl CatwalkBlaldeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (item->frameNumber == Anims[item->animNumber].frameEnd || frameNumber < 38)
			item->itemFlags[3] = 0;
		else
			item->itemFlags[3] = 100;

		AnimateItem(item);
	}
}

void __cdecl PlinthBladeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (item->frameNumber == Anims[item->animNumber].frameEnd)
			item->itemFlags[3] = 0;
		else
			item->itemFlags[3] = 200;

		AnimateItem(item);
	}
}

void __cdecl InitialiseSethBlade(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->animNumber = Objects[ID_SETH_BLADE].animIndex + 1;
	item->goalAnimState = 2;
	item->currentAnimState = 2;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->itemFlags[2] = abs(item->triggerFlags);
}

void __cdecl SethBladeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->itemFlags[0] = 0;
	if (TriggerActive(item))
	{
		if (item->currentAnimState == 2)
		{
			if (item->itemFlags[2] > 1)
			{
				item->itemFlags[2]--;
			}
			else if (item->itemFlags[2] == 1)
			{
				item->goalAnimState = 1;
				item->itemFlags[2] = 0;
			}
			else if (!item->itemFlags[2])
			{
				if (item->triggerFlags > 0)
				{
					item->itemFlags[2] = item->triggerFlags;
				}
			}
		}
		else
		{
			__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

			if (item->frameNumber != Anims[item->animNumber].frameBase && frameNumber <= 6)
			{
				item->itemFlags[0] = -1;
				item->itemFlags[3] = 1000;
			}
			else if (frameNumber >= 7 && frameNumber <= 15)
			{
				item->itemFlags[0] = 448;
				item->itemFlags[3] = 1000;
			}
			else
			{
				item->itemFlags[0] = 0;
				item->itemFlags[3] = 1000;
			}
		}

		AnimateItem(item);
	}
}

void __cdecl ChainControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->triggerFlags)
	{
		item->itemFlags[2] = 1;
		item->itemFlags[3] = 75;

		if (TriggerActive(item))
		{
			item->itemFlags[0] = 30846;
			AnimateItem(item);
			return;
		}
	}
	else
	{
		item->itemFlags[3] = 25;

		if (TriggerActive(item))
		{
			item->itemFlags[0] = 1920;
			AnimateItem(item);
			return;
		}
	}

	item->itemFlags[0] = 0;
}

void __cdecl PloughControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->itemFlags[3] = 50;
	if (TriggerActive(item))
	{
		item->itemFlags[0] = 258048;
		AnimateItem(item);
	}
	else
	{
		item->itemFlags[0] = 0;
	}
}

void __cdecl CogControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (TriggerActive(item))
	{
		item->status = ITEM_ACTIVE;
		// *(_DWORD *)&item->gap4C[5526] = *(_DWORD *)&item->gap4C[5526] & 0xFFFFFFFB | 2;
		AnimateItem(item);

		if (item->triggerFlags == 666)
		{
			PHD_VECTOR pos;
			GetJointAbsPosition(item, &pos, 0);
			SoundEffect(65, (PHD_3DPOS *)&pos, 0);

			if (item->frameNumber == Anims[item->animNumber].frameEnd)
				item->flags &= 0xC1;
		}
	}
	else if (item->triggerFlags == 2)
	{
		item->status |= ITEM_INVISIBLE;
	}
}

void __cdecl SpikeballControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (TriggerActive(item))
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if ((frameNumber <= 14 || frameNumber >= 24) && (frameNumber < 138 || frameNumber > 140))
		{
			if (frameNumber < 141)
				item->itemFlags[0] = 0;
			else
			{
				item->itemFlags[3] = 50;
				item->itemFlags[0] = 0x7FF800;
			}
		}
		else
		{
			item->itemFlags[3] = 150;
			item->itemFlags[0] = 0x7FF800;
		}

		AnimateItem(item);
	}
	else
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
}

