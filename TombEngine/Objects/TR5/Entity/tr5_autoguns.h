#pragma once

class Vector3i;

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseAutoGuns(short itemNumber);
	void AutoGunsControl(short itemNumber);
	void TriggerAutoGunSmoke(const Vector3i& pos, char shade);
}
