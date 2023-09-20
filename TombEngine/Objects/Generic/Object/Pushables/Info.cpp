#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Info.h"

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

	// TODO: Constants.
	PushableInfo::PushableInfo()
	{
		SoundState = PushableSoundState::None;
		Height = BLOCK(1);

		AnimationSystemIndex = 0;

		Gravity = 8.0f;
		BehaviourState = PushableState::Idle;
		WaterSurfaceHeight = NO_HEIGHT;
		FloatingForce = 0.75f;

		StackLimit = 3;
		StackUpperItem = -1;
		StackLowerItem = -1;

		CanFall = false;
		DoAlignCenter = true;
		IsBuoyant = false;
		UsesRoomCollision = false;
		BridgeColliderFlag = false;

		SidesMap =
		{
			{ 0, PushableSidesAttributes() }, // North
			{ 1, PushableSidesAttributes() }, // East
			{ 2, PushableSidesAttributes() }, // South
			{ 3, PushableSidesAttributes() }  // West
		};
	}
}
