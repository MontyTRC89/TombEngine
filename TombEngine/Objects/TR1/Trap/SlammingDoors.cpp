#include "framework.h"
#include "Objects/TR1/Trap/SlammingDoors.h"

#include "Game/control/Box.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Math/Objects/Vector3i.h"
#include "Math/Random.h"

using namespace TEN::Math;

namespace TEN::Entities::Traps::TR1
{
	constexpr auto SLAMMING_DOORS_DAMAGE = 400;
	constexpr auto LARA_RADIUS = 100;

	bool flagSpikeDoor = false;

	enum SlammingDoorsState
	{
		SLAMMINGDOORS_DISABLED = 0,
		SLAMMINGDOORS_ENABLED = 1
	};

	enum SlammingDoorsAnim
	{
		SLAMMINGDOORS_ANIM_OPENED = 0,
		SLAMMINGDOORS_ANIM_CLOSING = 1,
		SLAMMINGDOORS_ANIM_OPENING = 2
	};

	void InitialiseSlammingDoors(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		SetAnimation(&item, SLAMMINGDOORS_ANIM_OPENED);

		// used by GenericSphereBoxCollision, var where each bit means each damaging mesh index.
		// 3 = 000000000 000000011 so damage meshes are the 1 and 2 (both doors)
		item.ItemFlags[0] = 0;

		//used by GenericSphereBoxCollision, var for the trap damage value.
		item.ItemFlags[3] = SLAMMING_DOORS_DAMAGE;
	}

	void ControlSlammingDoors(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TriggerActive(&item))
		{
			if (item.Animation.TargetState != SLAMMINGDOORS_ENABLED)
			{
				item.Animation.TargetState = SLAMMINGDOORS_ENABLED;
				item.ItemFlags[0] = 3;
			}
		}
		else
		{
			if (item.Animation.TargetState != SLAMMINGDOORS_DISABLED)
			{
				item.Animation.TargetState = SLAMMINGDOORS_DISABLED;
				item.ItemFlags[0] = 0;
			}
		}

		AnimateItem(&item);
	}
}