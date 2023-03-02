#pragma once

namespace TEN::Entities::Generic
{
	enum class PushableMovementState
	{
		None,
		Moving,
		Stopping
	};
	
	struct PushableSidesAttributes
	{
		bool pullable;
		bool pushable;
		bool climbable;

		PushableSidesAttributes()
		{
			pullable = true;
			pushable = true;
			climbable = false;
		}

		PushableSidesAttributes(bool pullValue, bool pushValue, bool climbValue)
		{
			pullable = pullValue;
			pushable = pushValue;
			climbable = climbValue;
		}
	};

	struct PushableInfo
	{
		PushableMovementState MovementState = PushableMovementState::None;

		int height;				// height for collision, also in floor procedure
		int weight;
		int gravity;			// fall acceleration
		int animationGrab;		// the pull and push are sorted by states, so setting the right grab animation can make the system works.

		int stackLimit;			// max of pushables that can be over it so Lara can move it.
		int stackUpperItem;		// the itemNumber of the pushable that is placed over it.
		int stackLowerItem;		// the itemNumber of the pushable that is placed under it.

		GameVector StartPos;	// used for pushable movement code and to deactivate stopper flag
		
		std::map <int, PushableSidesAttributes> SidesMap;

		//flags
		bool canFall;			// OCB [0]
		bool doAlignCenter;		// OCB [1]
		bool hasFloorColission;	// per Slot? has floor and ceiling procedures (OCB 64)

		PushableInfo()
		{
			MovementState = PushableMovementState::None;
			height = BLOCK(1);
			weight = 100;
			gravity = 8;
			animationGrab = LA_PUSHABLE_GRAB;

			stackLimit = 3;
			stackUpperItem = -1;
			stackLowerItem = -1;

			canFall = false;
			doAlignCenter = true;
			hasFloorColission = false;
			SidesMap =
			{
				{0,	PushableSidesAttributes()},
				{1,	PushableSidesAttributes()},
				{2,	PushableSidesAttributes()},
				{3,	PushableSidesAttributes()}
			};
		}
	};
}
