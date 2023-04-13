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
		PushableSoundState	 CurrentSoundState = PushableSoundState::None;
		PushableGravityState GravityState	   = PushableGravityState::None;

		int	  Height		= 0;
		float Gravity		= 0.0f;
		float FloatingForce = 0.0f; // Oscillation strength while floating on water (recomended range: (0.0f - 2.0f]).
		
		int StackLimit			 = 0; // max of pushables that can be over it so Lara can move it.
		int StackUpperItem		 = 0; // the itemNumber of the pushable that is placed over it.
		int StackLowerItem		 = 0; // the itemNumber of the pushable that is placed under it.
		int AnimationSystemIndex = 0; // the index of the int PushableAnimationVector where are located the pull / push animation indices for this object.

		GameVector StartPos = GameVector::Zero;	// Used by movement code and to deactivate stopper flag.
		
		std::map<int, PushableSidesAttributes> SidesMap = {};

		bool CanFall		   = false; // OCB [0]: Can fall.
		bool DoAlignCenter	   = false; // OCB [1]: Player aligns to center when grabbing.
		bool IsBuoyant		   = false; // OCB [2]: Can float on water.
		bool UsesRoomCollision = false; // Per Slot: Uses room collision or object collision.
		
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
			IsBuoyant = false;
			UsesRoomCollision = false;
			
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
