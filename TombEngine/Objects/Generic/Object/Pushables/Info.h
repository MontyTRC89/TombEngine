#pragma once
#include "Math/Math.h"
#include "Objects/Generic/Object/Pushables/States.h"

using namespace TEN::Math;

namespace TEN::Entities::Generic
{
	enum class PushableSoundState
	{
		None,
		Move,
		Stop,
		Fall,
		Wade
	};

	struct PushableSidesAttributes
	{
		bool IsPullable	 = false;
		bool IsPushable	 = false;
		bool IsClimbable = false;

		PushableSidesAttributes();
		PushableSidesAttributes(bool isPullable, bool isPushable, bool isClimbable);
	};

	struct PushableInfo
	{
		PushableSoundState SoundState	  = PushableSoundState::None;
		PushableState	   BehaviourState = PushableState::Idle;

		int	  Height			 = 0;
		bool  IsOnEdge			 = false;
		float Gravity			 = 0.0f;
		int	  WaterSurfaceHeight = 0;	 // Used for spawning effects, e.g. water ripples.
		float FloatingForce		 = 0.0f; // Oscillation strength on water surface. Recomended range: (0.0f, 2.0f].
		
		int StackLimit			 = 0; // Max number of pushables in stack that can be pushed by player.
		int StackUpperItem		 = 0; // Item number of pushable above.
		int StackLowerItem		 = 0; // Item number of pushable below.
		int AnimationSystemIndex = 0; // ???Index of the int PushableAnimationVector where are located the pull / push animation indices for this object.

		GameVector StartPos = GameVector::Zero;	// XZ used by movement code, Y used to store water height level.

		std::map<int, PushableSidesAttributes> SidesMap = {};

		bool CanFall			= false; // OCB 0.
		bool DoAlignCenter		= false; // OCB 1.
		bool IsBuoyant			= false; // OCB 2.
		bool UsesRoomCollision	= false; // Use room collision or object collision. Per slot.
		bool UseBridgeCollision = false;

		PushableInfo();
	};
}
