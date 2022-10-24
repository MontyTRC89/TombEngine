#pragma once
#include "Math/Math.h"

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseHydra(short itemNumber);
	void HydraControl(short itemNumber);
	void TriggerHydraMissileSparks(Vector3i* pos, short xv, short yv, short zv);
}
