#include "framework.h"
#include "Game/Lara/lara_cheat.h"

#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_swim.h"
#include "Game/Setup.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"

using namespace TEN::Input;

namespace TEN::Entities::Player
{
	void lara_as_fly_cheat(ItemInfo* item, CollisionInfo* coll)
	{
		if (IsHeld(In::Forward))
		{
			item->Pose.Orientation.x -= ANGLE(3.0f);
		}
		else if (IsHeld(In::Back))
		{
			item->Pose.Orientation.x += ANGLE(3.0f);
		}

		if (IsHeld(In::Left))
		{
			ModulateLaraTurnRateY(item, ANGLE(3.4f), 0, ANGLE(6.0f));
		}
		else if (IsHeld(In::Right))
		{
			ModulateLaraTurnRateY(item, ANGLE(3.4f), 0, ANGLE(6.0f));
		}

		if (IsHeld(In::Action))
			TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 31, 150, 150, 150);

		if (IsHeld(In::Jump))
		{
			float velCoeff = IsHeld(In::Sprint) ? 2.5f : 1.0f;

			item->Animation.Velocity.y += (LARA_SWIM_VELOCITY_ACCEL * 4) * velCoeff;
			if (item->Animation.Velocity.y > (LARA_SWIM_VELOCITY_MAX * 2) * velCoeff)
				item->Animation.Velocity.y = (LARA_SWIM_VELOCITY_MAX * 2) * velCoeff;
		}
		else
		{
			if (item->Animation.Velocity.y >= LARA_SWIM_VELOCITY_ACCEL)
			{
				item->Animation.Velocity.y -= item->Animation.Velocity.y / 8;
			}
			else
			{
				item->Animation.Velocity.y = 0.0f;
			}
		}
	}

	// TODO: Avoid hooking into swim state.
	void lara_col_fly_cheat(ItemInfo* item, CollisionInfo* coll)
	{
		lara_col_underwater_swim_forward(item, coll);
	}
}
