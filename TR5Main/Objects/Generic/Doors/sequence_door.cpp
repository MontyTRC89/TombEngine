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
#include "Objects/Generic/Doors/sequence_door.h"
#include "Objects/Generic/Switches/fullblock_switch.h"
#include "Game/itemdata/door_data.h"

using namespace TEN::Entities::Switches;

namespace TEN::Entities::Doors
{
	void SequenceDoorControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		DOOR_DATA* door = (DOOR_DATA*)item->data;

		if (CurrentSequence == 3)
		{
			if (SequenceResults[Sequences[0]][Sequences[1]][Sequences[2]] == item->triggerFlags)
			{
				if (item->currentAnimState == 1)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 0;

				TestTriggers(item, true);
			}

			CurrentSequence = 4;
		}

		if (item->currentAnimState == item->goalAnimState)
		{
			if (item->currentAnimState == 1)
			{
				if (!door->opened)
				{
					OpenThatDoor(&door->d1, door);
					OpenThatDoor(&door->d2, door);
					OpenThatDoor(&door->d1flip, door);
					OpenThatDoor(&door->d2flip, door);
					door->opened = true;
					item->flags |= 0x3E;
				}
			}
			else
			{
				if (door->opened)
				{
					ShutThatDoor(&door->d1, door);
					ShutThatDoor(&door->d2, door);
					ShutThatDoor(&door->d1flip, door);
					ShutThatDoor(&door->d2flip, door);
					door->opened = false;
					item->flags &= 0xC1;
				}
			}
		}

		AnimateItem(item);
	}
}