#pragma once

class Vector3i;

namespace TEN::Entities::Creatures::TR5
{
	void InitializeHydra(short itemNumber);
	void HydraControl(short itemNumber);
	void TriggerHydraMissileSparks(const Vector3& pos, short xv, short yv, short zv);
}
