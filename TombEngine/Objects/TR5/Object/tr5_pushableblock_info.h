#pragma once

namespace TEN::Entities::Generic
{
	enum class PushableSoundState
	{
		None,
		Moving,
		Stopping
	};
	
	enum class PushableAnimationGroup
	{
		Statues,
		Blocks
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
		PushableSoundState soundState = PushableSoundState::None;

		int height;								// height for collision, also in floor procedure
		int gravity;							// fall acceleration
		
		int stackLimit;							// max of pushables that can be over it so Lara can move it.
		int stackUpperItem;						// the itemNumber of the pushable that is placed over it.
		int stackLowerItem;						// the itemNumber of the pushable that is placed under it.

		GameVector StartPos;	// used for pushable movement code and to deactivate stopper flag
		
		std::map <int, PushableSidesAttributes> SidesMap;

		//flags
		bool canFall;							// OCB [0]. flag to indicate if item can fall or not.
		bool doAlignCenter;						// OCB [1]. flag to decide if Lara has to put in center of the pushable to can move it.
		bool buoyancy;							// OCB [2]. flag to indicate if float in water.
		PushableAnimationGroup animationSystem;	// OCB [3]. flag to indicate which animations do.
		bool hasFloorColission;					// per Slot. flag to indicate if it uses floor data collision or object collision.

		PushableInfo()
		{
			soundState = PushableSoundState::None;
			height = BLOCK(1);

			gravity = 8;

			stackLimit = 3;
			stackUpperItem = -1;
			stackLowerItem = -1;

			canFall = false;
			doAlignCenter = true;
			buoyancy = false;
			animationSystem = PushableAnimationGroup::Statues;
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
