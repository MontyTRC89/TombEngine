#include "framework.h"
#include "Objects/TR5/Entity/tr5_willowwisp.h"

#include "Game/items.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::TR5
{
	enum WillowWispState
	{
		WWISP_STATE_UNK = 1
	};

	void InitialiseLightingGuide(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = WWISP_STATE_UNK;
		item->Animation.TargetState = WWISP_STATE_UNK;
	}
}
