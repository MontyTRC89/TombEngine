#pragma once

namespace TEN::Entities::Generic
{
	enum class PushableMovementState
	{
		None,
		Moving,
		Stopping
	};

	struct PushableInfo
	{
		PushableMovementState MovementState = PushableMovementState::None;

		int height;				// height for collision, also in floor procedure
		int weight;
		int stackLimit;
		int moveX;				// used for pushable movement code
		int moveZ;				// used for pushable movement code
		int linkedIndex;		// using itemFlags[1] for now
		int gravity;			// fall acceleration
		int loopSound;			// looped sound index for movement
		int stopSound;			// ending sound index
		int fallSound;			// sound on hitting floor (if dropped)
		int climb;				// not used for now
		bool canFall;			// OCB 32
		bool hasFloorCeiling;	// has floor and ceiling procedures (OCB 64)
		bool disablePull;		// OCB 128
		bool disablePush;		// OCB 256
		bool disableW;			// OCB 512 (W+E)
		bool disableE;			// OCB 512 (W+E)
		bool disableN;			// OCB 1024 (N+S)
		bool disableS;			// OCB 1024 (N+S)
	};
}
