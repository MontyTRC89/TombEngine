#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::Switches
{
	extern byte SequenceUsed[6];
	extern byte SequenceResults[3][3][3];
	extern byte Sequences[3];
	extern byte CurrentSequence;

	void FullBlockSwitchControl(short itemNumber);
	void FullBlockSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
}
