#include "newobjects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\people.h"
#include "..\Game\effects.h"
#include "..\Game\effect2.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"
#include "..\Game\inventory.h"
#include "..\Game\collide.h"
#include "..\Game\draw.h"

void __cdecl InitialiseJeanYves(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	OBJECT_INFO* obj = &Objects[item->objectNumber];
	
	item->goalAnimState = 1;
	item->currentAnimState = 1;
	item->animNumber = obj->animIndex;
	item->frameNumber = Anims[item->animNumber].frameBase;

}

void __cdecl JeanYvesControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->triggerFlags >= Lara.highestLocation)
	{
		__int16 state = 0;

		if (GetRandomControl() & 3)
		{
			state = (GetRandomControl() & 1) + 1;
		}
		else
		{
			state = 3 * (GetRandomControl() & 1);
		}

		item->goalAnimState = (((byte)(item->currentAnimState) - 1) & 0xC) + state + 1;
		
		AnimateItem(item);
	}
	else
	{
		if (Lara.highestLocation > 3)
			Lara.highestLocation = 3;

		__int16 state = (GetRandomControl() & 3) + 4 * Lara.highestLocation;
		__int16 animNumber = Objects[item->objectNumber].animIndex + state;
		state++;

		item->goalAnimState = item->currentAnimState = state;
		item->animNumber = animNumber;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->triggerFlags = Lara.highestLocation;

		AnimateItem(item);
	}
}