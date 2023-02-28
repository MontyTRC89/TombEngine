#include "framework.h"
#include "Objects/TR1/Trap/SlammingDoors.h"

#include "Game/control/Box.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Math/Objects/Vector3i.h"

using namespace TEN::Math;

namespace TEN::Entities::Traps::TR1
{
	constexpr auto SLAMMING_DOORS_DAMAGE = 400;

	const auto Teeth1ABite = BiteInfo(Vector3(-23.0f, 0.0f, -1718.0f), 0);
	const auto Teeth1BBite = BiteInfo(Vector3(71.0f, 0.0f, -1718.0f), 1);
	const auto Teeth2ABite = BiteInfo(Vector3(-23.0f, 10.0f, -1718.0f), 0);
	const auto Teeth2BBite = BiteInfo(Vector3(71.0f, 10.0f, -1718.0f), 1);
	const auto Teeth3ABite = BiteInfo(Vector3(-23.0f, -10.0f, -1718.0f), 0);
	const auto Teeth3BBite = BiteInfo(Vector3(71.0f, -10.0f, -1718.0f), 1);

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
				DoDamage(LaraItem, SLAMMING_DOORS_DAMAGE);
				DoBiteEffect(item, Teeth1ABite);
				DoBiteEffect(item, Teeth1BBite);
				DoBiteEffect(item, Teeth2ABite);
				DoBiteEffect(item, Teeth2BBite);
				DoBiteEffect(item, Teeth3ABite);
				DoBiteEffect(item, Teeth3BBite);
			}
		}
		else
		{
			item->Animation.TargetState = SLAMMINGDOORS_DISABLED;
		}

		AnimateItem(item);
	}

	void DoBiteEffect(ItemInfo* item, const BiteInfo& bite)
	{
		auto pos = GetJointPosition(item, bite.meshNum) + bite.Position;

		//item->Animation.Velocity.z returned 0 so I put 1 for velocity. Kubsy 27/02/2023
		DoBloodSplat(pos.x, pos.y, pos.z, 1, item->Pose.Orientation.y, item->RoomNumber);
	}
}