#pragma once

namespace TEN::Entities::Generic
{
	enum class PushablePhysicState
	{
		Idle,
		Moving,
		Falling,
		Sinking,
		Floating,
		UnderwaterIdle,
		WatersurfaceIdle,
		Sliding
	};

	extern std::unordered_map<PushablePhysicState, std::function<void(int)>> PUSHABLES_STATES_MAP;

	void InitializePushablesStatesMap();

	void HandleIdleState(int itemNumber);
	void HandleMovingState(int itemNumber);
	void HandleFallingState(int itemNumber);
	void HandleSinkingState(int itemNumber);
	void HandleFloatingState(int itemNumber);
	void HandleUnderwaterState(int itemNumber);
	void HandleWatersurfaceState(int itemNumber);
	void HandleSlidingState(int itemNumber);
};
