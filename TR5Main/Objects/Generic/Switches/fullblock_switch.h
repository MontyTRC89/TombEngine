#pragma once

#include "items.h"
#include <collide.h>

namespace TEN::Entities::Switches
{
	extern byte SequenceUsed[6];
	extern byte SequenceResults[3][3][3];
	extern byte Sequences[3];
	extern byte CurrentSequence;

	void FullBlockSwitchControl(short itemNumber);
	void FullBlockSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}