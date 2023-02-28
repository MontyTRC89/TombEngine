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
	}

	void ControlSlammingDoors(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			item->Animation.TargetState = SLAMMINGDOORS_ENABLED;

			if (item->TouchBits.TestAny() && item->Animation.ActiveState == SLAMMINGDOORS_ENABLED)
			{
				int x = LaraItem->Pose.Position.x + Random::GenerateInt(-LARA_RADIUS, LARA_RADIUS);
				int y = LaraItem->Pose.Position.y - Random::GenerateInt(CLICK(1), CLICK(3));
				int z = LaraItem->Pose.Position.z + Random::GenerateInt(-LARA_RADIUS, LARA_RADIUS);

				DoDamage(LaraItem, SLAMMING_DOORS_DAMAGE);
				DoBloodSplat(x, 
					y, 
					z, 
					Random::GenerateFloat(-10.0f, 10.0f), 
					LaraItem->Pose.Orientation.y, 
					LaraItem->RoomNumber);
			}
		}
		else
		{
			item->Animation.TargetState = SLAMMINGDOORS_DISABLED;
		}

		AnimateItem(item);
	}
}