#pragma once

class Vector3i;

namespace TEN::Entities::Generic
{
	enum class PushableEnvironmentType
	{
		Air,
		FlatFloor,
		SlopedFloor,
		Water,
		WaterFloor
	};

	bool IsPushableValid(int itemNumber);
	bool IsPushableObjectMoveAllowed(int itemNumber, const Vector3i& targetPos, int targetRoom);
	bool IsValidForPlayer(int itemNumber);

	bool PushableIdleConditions(int itemNumber);
	bool PushableMovementConditions(int itemNumber, bool hasPushAction, bool hasPullAction);

	PushableEnvironmentType GetPushableEnvironmentType(int itemNumber, int& floorHeight, int* ceilingHeight = nullptr);
}
