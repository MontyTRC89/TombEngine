#pragma once

struct ItemInfo;

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

	void HandlePushableBehaviorState(ItemInfo& pushableItem);

	void HandleIdleState(ItemInfo& pushableItem);
	void HandleMoveState(ItemInfo& pushableItem);
	void HandleEdgeSlipState(ItemInfo& pushableItem);
	void HandleFallState(ItemInfo& pushableItem);
	void HandleSinkState(ItemInfo& pushableItem);
	void HandleFloatState(ItemInfo& pushableItem);
	void HandleUnderwaterState(ItemInfo& pushableItem);
	void HandleWaterSurfaceState(ItemInfo& pushableItem);
	void HandleSlideState(ItemInfo& pushableItem);
	void HandleMoveStackHorizontalState(ItemInfo& pushableItem);
};
