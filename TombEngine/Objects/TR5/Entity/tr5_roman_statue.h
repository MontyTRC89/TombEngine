#pragma once
#include "Specific/phd_global.h"

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseRomanStatue(short itemNumber);
	void RomanStatueControl(short itemNumber);
	void TriggerRomanStatueMissileSparks(Vector3Int* pos, char fxObject);
}
