#include "framework.h"
#include "generic_doors.h"
#include "level.h"
#include "control.h"
#include "box.h"
#include "items.h"
#include "lot.h"
#include "newinv2.h"
#include "input.h"
#include "pickup.h"
#include "sound.h"
#include "draw.h"
#include "sphere.h"
#include "lara_struct.h"
#include "lara.h"
#include "trmath.h"
#include "misc.h"
#include "sequence_door.h"
#include "fullblock_switch.h"

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

				TestTriggers(item, true, NULL);
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