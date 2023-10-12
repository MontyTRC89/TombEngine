#include "framework.h"
#include "Objects/Generic/Object/Pushable/Info.h"

#include "Game/control/control.h"

namespace TEN::Entities::Generic
{
	const auto PUSHABLE_SIDES_ATTRIBUTES_DEFAULT = PushableSidesAttributes(true, true, false);

	PushableSidesAttributes::PushableSidesAttributes()
	{
		*this = PUSHABLE_SIDES_ATTRIBUTES_DEFAULT;
	}

	PushableSidesAttributes::PushableSidesAttributes(bool isPullable, bool isPushable, bool isClimbable)
	{
		IsPullable = isPullable;
		IsPushable = isPushable;
		IsClimbable = isClimbable;
	}

	// TODO: Define constants.
	PushableInfo::PushableInfo()
	{
		SoundState = PushableSoundState::None;
		Height = BLOCK(1);

		AnimationSystemIndex = 0;

		Gravity = 8.0f;
		BehaviorState = PushableState::Idle;
		WaterSurfaceHeight = NO_HEIGHT;
		FloatingForce = 0.75f;

		StackLimit = 3;
		StackUpperItem = -1;
		StackLowerItem = -1;

		CanFall = false;
		DoAlignCenter = true;
		IsBuoyant = false;
		UseRoomCollision = false;
		UseBridgeCollision = false;

		// TODO: Descriptive name.
		SidesMap =
		{
			{ CardinalDirection::NORTH, PushableSidesAttributes() },
			{ CardinalDirection::EAST, PushableSidesAttributes() },
			{ CardinalDirection::SOUTH, PushableSidesAttributes() },
			{ CardinalDirection::WEST, PushableSidesAttributes() }
		};
	}
}
