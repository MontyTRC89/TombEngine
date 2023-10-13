#pragma once

class Vector3i;
struct ItemInfo;

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

	struct PushableCollisionData
	{
		PushableEnvironmentType EnvType = PushableEnvironmentType::Air;

		int FloorHeight	  = 0;
		int CeilingHeight = 0;
	};

	bool IsPushableValid(int itemNumber);
	bool IsPushableObjectMoveAllowed(int itemNumber, const Vector3i& targetPos, int targetRoom);
	bool IsValidForPlayer(int itemNumber);

	bool PushableIdleConditions(int itemNumber);
	bool PushableMovementConditions(int itemNumber, bool hasPushAction, bool hasPullAction);

	PushableCollisionData GetPushableCollision(const ItemInfo& item);
}
