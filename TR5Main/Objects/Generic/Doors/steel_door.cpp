#include "framework.h"
#include "Objects/Generic/Doors/generic_doors.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Game/gui.h"
#include "Specific/input.h"
#include "Game/pickup/pickup.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/collision/sphere.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara.h"
#include "Specific/trmath.h"
#include "Game/misc.h"
#include "Objects/Generic/Doors/steel_door.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"

namespace TEN::Entities::Doors
{
	void InitialiseSteelDoor(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		item->MeshBits = 1;
		item->Position.yPos -= 1024;
	}

	void SteelDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->ItemFlags[0] != 3)
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