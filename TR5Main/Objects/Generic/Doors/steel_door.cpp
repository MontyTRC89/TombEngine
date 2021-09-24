#include "framework.h"
#include "generic_doors.h"
#include "level.h"
#include "control/control.h"
#include "control/box.h"
#include "items.h"
#include "control/lot.h"
#include "newinv2.h"
#include "input.h"
#include "pickup.h"
#include "sound.h"
#include "animation.h"
#include "sphere.h"
#include "lara_struct.h"
#include "lara.h"
#include "trmath.h"
#include "misc.h"
#include "steel_door.h"
#include "collide.h"
#include "item.h"
namespace TEN::Entities::Doors
{
	void InitialiseSteelDoor(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		item->meshBits = 1;
		item->pos.yPos -= 1024;
	}

	void SteelDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->itemFlags[0] != 3)
		{
			if (TestBoundsCollide(item, l, coll->Setup.Radius))
			{
				if (TestCollision(item, l))
				{
					if (coll->Setup.EnableObjectPush)
						ItemPushItem(item, l, coll, 0, 1);
				}
			}
		}
	}
}