#pragma once

namespace TEN::Entities::Generic
{
	void ActivateClimbablePushableCollider(int itemNumber);
	void DeactivateClimbablePushableCollider(int itemNumber);
	void RefreshClimbablePushableCollider(int itemNumber);

	std::optional<int> ClimbablePushableFloor(int itemNumber, int x, int y, int z);
	std::optional<int> ClimbablePushableCeiling(int itemNumber, int x, int y, int z);
	int ClimbablePushableFloorBorder(int itemNumber);
	int ClimbablePushableCeilingBorder(int itemNumber);
}
