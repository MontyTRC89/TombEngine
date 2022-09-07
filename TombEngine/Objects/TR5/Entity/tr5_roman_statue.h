#pragma once
#include "Math/Math.h"

namespace TEN::Entities::TR5
{
	void InitialiseRomanStatue(short itemNumber);
	void RomanStatueControl(short itemNumber);
	void TriggerRomanStatueMissileSparks(Vector3i* pos, char fxObject);
}
