#pragma once

namespace TEN::Entities::Generic
{
	enum class PushableState
	{
		Idle,
		Move,
		EdgeSlip,
		Fall,
		Sink,
		Float,
		UnderwaterIdle,
		WaterSurfaceIdle,
		Slide,
		MoveStackHorizontal
	};

	extern std::unordered_map<PushableState, std::function<void(int)>> PUSHABLE_STATE_MAP;
	
	void InitializePushableStateMap();

	void HandleIdleState(int itemNumber);
	void HandleMoveState(int itemNumber);
	void HandleEdgeSlipState(int itemNumber);
	void HandleFallState(int itemNumber);
	void HandleSinkState(int itemNumber);
	void HandleFloatState(int itemNumber);
	void HandleUnderwaterState(int itemNumber);
	void HandleWaterSurfaceState(int itemNumber);
	void HandleSlideState(int itemNumber);
	void HandleMoveStackHorizontalState(int itemNumber);
};
