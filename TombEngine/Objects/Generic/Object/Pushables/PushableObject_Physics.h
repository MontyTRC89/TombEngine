#pragma once

namespace TEN::Entities::Generic
{
	enum class PushablePhysicState
	{
		Idle,
		Moving,
		MovingEdge,
		Falling,
		Sinking,
		Floating,
		UnderwaterIdle,
		WatersurfaceIdle,
		Sliding,
		StackHorizontalMove
	};

	extern std::unordered_map<PushablePhysicState, std::function<void(int)>> PUSHABLES_STATES_MAP;

	void InitializePushablesStatesMap();

	void HandleIdleState(int itemNumber);
	void HandleMovingState(int itemNumber);
	void HandleMovingEdgeState(int itemNumber);
	void HandleFallingState(int itemNumber);
	void HandleSinkingState(int itemNumber);
	void HandleFloatingState(int itemNumber);
	void HandleUnderwaterState(int itemNumber);
	void HandleWatersurfaceState(int itemNumber);
	void HandleSlidingState(int itemNumber);
	void HandleStackHorizontalMoveState(int itemNumber);
};
