#include "framework.h"
#include "Objects/TR1/Trap/TeethSpikeDoor.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Entities::Traps::TR1
{
	constexpr auto TEETH_SPIKE_DOOR_DAMAGE = 400;

	enum TeethSpikeDoorState
	{
		TEETHSPIKEDOOR_DISABLED = 0,
		TEETHSPIKEDOOR_ENABLED = 1
	};

	void InitialiseTeethSpikeDoor(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
	}

	void ControlTeethSpikeDoor(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			item->Animation.ActiveState = item->Animation.TargetState = TEETHSPIKEDOOR_ENABLED;
			AnimateItem(item);

			if (item->TouchBits.TestAny() && item->Animation.ActiveState == TEETHSPIKEDOOR_ENABLED)
			{
				DoDamage(LaraItem, TEETH_SPIKE_DOOR_DAMAGE);
				//TODO Add blood and check for individual spike in door(?)
			}
		}
		else
		{
			item->Animation.ActiveState = item->Animation.TargetState = TEETHSPIKEDOOR_DISABLED;
		}
	}
}