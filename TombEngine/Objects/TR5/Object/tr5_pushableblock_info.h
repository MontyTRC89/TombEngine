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
		int gravity;			// fall acceleration
		
		int stackLimit;
		int stackItemNumber;			// 

		GameVector StartPos;	// used for pushable movement code and to deactivate stopper flag
		
		int loopSound;			// looped sound index for movement
		int stopSound;			// ending sound index
		int fallSound;			// sound on hitting floor (if dropped)

		int climb;				// not used for now
		bool canFall;			// OCB 32
		bool doAlignCenter;		// --
		bool hasFloorColission;	// has floor and ceiling procedures (OCB 64)
		bool disablePull;		// OCB 128
		bool disablePush;		// OCB 256
		bool disableW;			// OCB 512 (W+E)
		bool disableE;			// OCB 512 (W+E)
		bool disableN;			// OCB 1024 (N+S)
		bool disableS;			// OCB 1024 (N+S)
	};
}
