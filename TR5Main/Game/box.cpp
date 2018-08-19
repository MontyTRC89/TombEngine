#include "Box.h"
#include "..\Global\global.h"
#include "items.h"
#include "tomb4fx.h"
#include "lot.h"
#include "deltapak.h"

/*void __cdecl CreatureDie(__int16 itemNum, __int32 explode)
{
	ITEM_INFO* item = &Items[itemNum];

	item->hitPoints = -16384;
	item->collidable = false;

	if (explode)
	{
		if (Objects[item->objectNumber].hitEffect == 1)
			ExplodingDeath2(itemNum, -1, 258);
		else
			ExplodingDeath2(itemNum, -1, 256);

		KillItem(itemNum);
	}
	else
		RemoveActiveItem(itemNum);

	DisableBaddieAI(itemNum);
	item->flags |= IFLAG_KILLED;
	DropBaddyPickups(item);

	if (item->objectNumber == ID_SCIENTIST && item->aiBits == 20)
	{
		item = FindItem(ID_ROLLINGBALL);
		if (item)
		{
			if (!(item->flags & IFLAG_INVISIBLE))
			{
				item->flags |= IFLAG_ACTIVATION_MASK;
				AddActiveItem(item - Items);
			}
		}
	}
}*/

void Inject_Box()
{
	//INJECT(0x0040A090, CreatureDie);
}