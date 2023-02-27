#include "framework.h"
#include "Objects/TR1/Trap/TeethSpikeDoor.h"

#include "Game/control/Box.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Math/Objects/Vector3i.h"

using namespace TEN::Math;

namespace TEN::Entities::Traps::TR1
{
	constexpr auto TEETH_SPIKE_DOOR_DAMAGE = 400;

	const auto Teeth1ABite = BiteInfo(Vector3(-23.0f, 0.0f, -1718.0f), 0);
	const auto Teeth1BBite = BiteInfo(Vector3(71.0f, 0.0f, -1718.0f), 1);
	const auto Teeth2ABite = BiteInfo(Vector3(-23.0f, 10.0f, -1718.0f), 0);
	const auto Teeth2BBite = BiteInfo(Vector3(71.0f, 10.0f, -1718.0f), 1);
	const auto Teeth3ABite = BiteInfo(Vector3(-23.0f, -10.0f, -1718.0f), 0);
	const auto Teeth3BBite = BiteInfo(Vector3(71.0f, -10.0f, -1718.0f), 1);

	enum TeethSpikeDoorState
	{
		TEETHSPIKEDOOR_DISABLED = 0,
		TEETHSPIKEDOOR_ENABLED = 1
	};

	enum TeethSpikeDoorAnim
	{
		TEETHSPIKEDOOR_ANIM_OPENED = 0,
		TEETHSPIKEDOOR_ANIM_CLOSING = 1,
		TEETHSPIKEDOOR_ANIM_OPENING = 2
	};

	void InitialiseTeethSpikeDoor(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		SetAnimation(&item, TEETHSPIKEDOOR_ANIM_OPENED);
	}

	void ControlTeethSpikeDoor(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			item->Animation.TargetState = TEETHSPIKEDOOR_ENABLED;

			if (item->TouchBits.TestAny() && item->Animation.ActiveState == TEETHSPIKEDOOR_ENABLED)
			{
				DoDamage(LaraItem, TEETH_SPIKE_DOOR_DAMAGE);
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
			item->Animation.TargetState = TEETHSPIKEDOOR_DISABLED;
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