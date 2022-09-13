#pragma once
#include "Specific/phd_global.h"

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseHydra(short itemNumber);
	void HydraControl(short itemNumber);
	void TriggerHydraMissileSparks(Vector3Int* pos, short xv, short yv, short zv);
}
