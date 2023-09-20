#pragma once

class Vector3i;

namespace TEN::Entities::Generic
{
	enum class PushableEnvironmentState
	{
		Air,
		Ground,
		Slope,
		Water,
		GroundWater
	};

	bool IsPushableValid(int itemNumber);
	bool IsPushableObjectMoveAllowed(int itemNumber, const Vector3i& targetPos, int targetRoom);
	bool IsValidForPlayer(int itemNumber);

	bool PushableIdleConditions(int itemNumber);
	bool PushableMovementConditions(int itemNumber, bool hasPushAction, bool hasPullAction);

	PushableEnvironmentState CheckPushableEnvironment(int itemNumber, int& floorHeight, int* ceilingHeight = nullptr);
}
