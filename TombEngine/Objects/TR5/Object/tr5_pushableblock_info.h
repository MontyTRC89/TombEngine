#pragma once

namespace TEN::Entities::Generic
{
	enum class PushableSoundState
	{
		None,
		Moving,
		Stopping
	};

	enum class PushableGravityState
	{
		None,
		Ground,
		Falling,
		Sinking,
		Floating,
		OnWater,
		Sliding //TODO.
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
		float Gravity;							// fall acceleration
		PushableGravityState GravityState;
		
		int StackLimit;							// max of pushables that can be over it so Lara can move it.
		int StackUpperItem;						// the itemNumber of the pushable that is placed over it.
		int StackLowerItem;						// the itemNumber of the pushable that is placed under it.
		int AnimationSystemIndex;				// the index of the int PushableAnimationVector where are located the pull / push animation indices for this object.

		GameVector StartPos;	// used for pushable movement code and to deactivate stopper flag
		
		std::map <int, PushableSidesAttributes> SidesMap;

		//flags
		bool CanFall;							// OCB [0]. flag to indicate if item can fall or not.
		bool DoAlignCenter;						// OCB [1]. flag to decide if Lara has to put in center of the pushable to can move it.
		bool Buoyancy;							// OCB [2]. flag to indicate if float in water.
		bool HasFloorColission;					// per Slot. flag to indicate if it uses floor data collision or object collision.
		
		//constructor
		PushableInfo()
		{
			CurrentSoundState = PushableSoundState::None;
			Height = BLOCK(1);

			AnimationSystemIndex = 0;

			Gravity = 8.0f;
			GravityState = PushableGravityState::Ground;

			StackLimit = 3;
			StackUpperItem = -1;
			StackLowerItem = -1;

			CanFall = false;
			DoAlignCenter = true;
			Buoyancy = false;
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
