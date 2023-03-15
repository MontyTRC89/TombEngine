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
		Grounded,
		Falling,
		Sinking,
		Floating,
		OnWater,
		Sliding // TODO
	};

	struct PushableSidesAttributes
	{
		bool Pullable  = false;
		bool Pushable  = false;
		bool Climbable = false;

		PushableSidesAttributes()
		{
			Pullable = true;
			Pushable = true;
			Climbable = false;
		}

		PushableSidesAttributes(bool isPullable, bool isPushable, bool isClimbable)
		{
			Pullable = isPullable;
			Pushable = isPushable;
			Climbable = isClimbable;
		}
	};
	
	struct PushableInfo
	{
		PushableSoundState CurrentSoundState = PushableSoundState::None;

		int Height;								// height for collision, also in floor procedure
		float Gravity;							// fall acceleration
		PushableGravityState GravityState;
		float FloatingForce;					// how strong are the oscilations while the object is floating in water. (recomended range 0.0f - 2.0f)
		
		int StackLimit;							// max of pushables that can be over it so Lara can move it.
		int StackUpperItem;						// the itemNumber of the pushable that is placed over it.
		int StackLowerItem;						// the itemNumber of the pushable that is placed under it.
		int AnimationSystemIndex;				// the index of the int PushableAnimationVector where are located the pull / push animation indices for this object.

		GameVector StartPos;	// used for pushable movement code and to deactivate stopper flag
		
		std::map<int, PushableSidesAttributes> SidesMap;

		// Flags
		bool CanFall;							// OCB [0]. flag to indicate if item can fall or not.
		bool DoAlignCenter;						// OCB [1]. flag to decide if Lara has to put in center of the pushable to can move it.
		bool Buoyancy;							// OCB [2]. flag to indicate if float in water.
		bool HasFloorColission;					// per Slot. flag to indicate if it uses floor data collision or object collision.
		
		// Constructor
		PushableInfo()
		{
			CurrentSoundState = PushableSoundState::None;
			Height = BLOCK(1);

			AnimationSystemIndex = 0;

			Gravity = 8.0f;
			GravityState = PushableGravityState::Grounded;
			FloatingForce = 0.75f;

			StackLimit = 3;
			StackUpperItem = -1;
			StackLowerItem = -1;

			CanFall = false;
			DoAlignCenter = true;
			Buoyancy = false;
			HasFloorColission = false;
			
			SidesMap =
			{
				{ 0, PushableSidesAttributes() },
				{ 1, PushableSidesAttributes() },
				{ 2, PushableSidesAttributes() },
				{ 3, PushableSidesAttributes() }
			};
		}
	};
}
