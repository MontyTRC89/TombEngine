#pragma once

class Vector3i;

namespace TEN::Entities::Generic
{
	bool IsPushableValid(int itemNumber);
	bool IsPushableObjectMoveAllowed(int itemNumber, Vector3i targetPos, int targetRoom);
	bool IsValidForLara(int itemNumber);
	bool PushableMovementConditions(int itemNumber, bool hasPushAction, bool hasPullAction);
}
