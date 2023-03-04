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
		bool Pullable;
		bool Pushable;
		bool Climbable;

		PushableSidesAttributes()
		{
			Pullable = true;
			Pushable = true;
			Climbable = false;
		}

		PushableSidesAttributes(bool pullValue, bool pushValue, bool climbValue)
		{
			Pullable = pullValue;
			Pushable = pushValue;
			Climbable = climbValue;
		}
	};

	struct PushableInfo
	{
		PushableSoundState CurrentSoundState = PushableSoundState::None;

		int Height;								// height for collision, also in floor procedure
		int Gravity;							// fall acceleration
		
		int StackLimit;							// max of pushables that can be over it so Lara can move it.
		int StackUpperItem;						// the itemNumber of the pushable that is placed over it.
		int StackLowerItem;						// the itemNumber of the pushable that is placed under it.

		GameVector StartPos;	// used for pushable movement code and to deactivate stopper flag
		
		std::map <int, PushableSidesAttributes> SidesMap;

		//flags
		bool CanFall;							// OCB [0]. flag to indicate if item can fall or not.
		bool DoAlignCenter;						// OCB [1]. flag to decide if Lara has to put in center of the pushable to can move it.
		bool Buoyancy;							// OCB [2]. flag to indicate if float in water.
		PushableAnimationGroup AnimationSystem;	// OCB [3]. flag to indicate which animations do.
		bool HasFloorColission;					// per Slot. flag to indicate if it uses floor data collision or object collision.

		PushableInfo()
		{
			CurrentSoundState = PushableSoundState::None;
			Height = BLOCK(1);

			Gravity = 8;

			StackLimit = 3;
			StackUpperItem = -1;
			StackLowerItem = -1;

			CanFall = false;
			DoAlignCenter = true;
			Buoyancy = false;
			AnimationSystem = PushableAnimationGroup::Statues;
			HasFloorColission = false;
			
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
