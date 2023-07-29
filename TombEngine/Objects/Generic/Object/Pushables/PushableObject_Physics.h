#pragma once

namespace TEN::Entities::Generic
{

	/*constexpr auto PUSHABLE_FALL_VELOCITY_MAX = BLOCK(1 / 8.0f);
	constexpr auto PUSHABLE_WATER_VELOCITY_MAX = BLOCK(1 / 16.0f);
	constexpr auto PUSHABLE_FALL_RUMBLE_VELOCITY = 96.0f;
	constexpr auto PUSHABLE_HEIGHT_TOLERANCE = 32;

	constexpr auto GRAVITY_AIR = 8.0f;
	constexpr auto GRAVITY_ACCEL = 0.5f;
	constexpr auto WATER_SURFACE_DISTANCE = CLICK(0.5f);*/

	enum class PushablePhysicState
	{
		Idle,
		Moving,
		Falling,
		Sinking,
		Floating,
		Underwater,
		Watersurface,
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
