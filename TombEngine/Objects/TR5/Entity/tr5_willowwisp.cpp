#include "framework.h"
#include "Objects/TR5/Entity/tr5_willowwisp.h"

#include "control/box.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::Creatures::TR5
{
	enum WillowWispState
	{
		WWISP_STATE_UNK = 1
	};

	void InitialiseLightingGuide(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);
		SetAnimation(item, 0);
	}
}
