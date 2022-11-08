#pragma once

class Vector3i;

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseRomanStatue(short itemNumber);
	void RomanStatueControl(short itemNumber);
	void TriggerRomanStatueMissileSparks(Vector3i* pos, char fxObject);
}
