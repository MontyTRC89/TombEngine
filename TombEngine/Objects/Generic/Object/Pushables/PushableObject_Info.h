#pragma once

#include "Objects/Generic/Object/Pushables/PushableObject_Physics.h"

namespace TEN::Entities::Generic
{

	enum class PushableSoundState
	{
		None,
		Moving,
		Stopping,
		Falling,
		WaterRipples
	};

	struct PushableSidesAttributes
	{
		bool Pullable = false;
		bool Pushable = false;
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

	typedef PushablePhysicState PushablePhysicState;

	struct PushableInfo
	{
		PushableSoundState CurrentSoundState = PushableSoundState::None;
		PushablePhysicState BehaviourState = PushablePhysicState::Idle;

		int	  Height = 0;
		bool isOnEdge = false;
		float Gravity = 0.0f;
		int WaterSurfaceHeight = 0;		//Used to spawn effects. (Like water ripples).
		float FloatingForce = 0.0f;		// Oscillation strength while floating on water (recomended range: (0.0f - 2.0f]).
		
		int StackLimit = 0;				// max of pushables that can be over it so Lara can move it.
		int StackUpperItem = 0;			// the itemNumber of the pushable that is placed over it.
		int StackLowerItem = 0;			// the itemNumber of the pushable that is placed under it.
		int AnimationSystemIndex = 0;	// the index of the int PushableAnimationVector where are located the pull / push animation indices for this object.

		GameVector StartPos = GameVector::Zero;	// XZ used by movement code, Y used to store water height level.

		std::map<int, PushableSidesAttributes> SidesMap = {};

		bool CanFall = false;			// OCB [0]: Can fall.
		bool DoAlignCenter = false;		// OCB [1]: Player aligns to center when grabbing.
		bool IsBuoyant = false;			// OCB [2]: Can float on water.
		bool UsesRoomCollision = false; // Per Slot: Uses room collision or object collision.
		bool BridgeColliderFlag = false;

		PushableInfo()
		{
			CurrentSoundState = PushableSoundState::None;
			Height = BLOCK(1);

			AnimationSystemIndex = 0;

			Gravity = 8.0f;
			BehaviourState = PushablePhysicState::Idle;
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
				{ 0, PushableSidesAttributes() },	//NORTH
				{ 1, PushableSidesAttributes() },	//EAST
				{ 2, PushableSidesAttributes() },	//SOUTH
				{ 3, PushableSidesAttributes() }	//WEST
			};
		}

	};
}
