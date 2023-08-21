#pragma once

class Vector3i;

namespace TEN::Entities::Generic
{
	enum class PushableEnvironemntState
	{
		Air,
		Ground,
		Slope,
		ShallowWater,
		DeepWater
	};

	bool IsPushableValid(int itemNumber);
	bool IsPushableObjectMoveAllowed(int itemNumber, Vector3i targetPos, int targetRoom);
	bool IsValidForLara(int itemNumber);

	bool PushableIdleConditions(int itemNumber);
	bool PushableMovementConditions(int itemNumber, bool hasPushAction, bool hasPullAction);

	PushableEnvironemntState CheckPushableEnvironment(int itemNumber, int& floorHeight);
}
