#include "newobjects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"
#include "..\Game\effect2.h"
#include "..\Game\people.h"
#include "..\Game\debris.h"

void __cdecl InitialiseKnightTemplar(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[ID_KNIGHT_TEMPLAR].animIndex + 2;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->meshBits &= 0xF700;
}

void __cdecl KnightTemplarControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	if (item->animNumber == obj->animIndex ||
		item->animNumber - obj->animIndex == 1 ||
		item->animNumber - obj->animIndex == 11 ||
		item->animNumber - obj->animIndex == 12)
	{
		if (GetRandomControl() & 1)
		{
			PHD_VECTOR pos;

			pos.x = 0;
			pos.y = 48;
			pos.z = 448;

			GetJointAbsPosition(item, &pos, 10);

			/*v4 = (GetRandomControl() & 0x1FF) - 256;
			v5 = -128 - (GetRandomControl() & 0x7F);
			v6 = GetRandomControl();
			sub_434200(v24, v25, v26, (v6 & 0x1FF) - 256, v5, v4, 0);
		*/}

	}
}