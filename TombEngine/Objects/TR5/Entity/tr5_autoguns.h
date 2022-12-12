#pragma once

class Vector3i;
struct ObjectInfo;

namespace TEN::Entities::Creatures::TR5
{
	void SetupAutoGun(ObjectInfo& object);
	void InitialiseAutoGuns(short itemNumber);

	void ControlAutoGun(short itemNumber);
	void SpawnAutoGunSmoke(const Vector3i& pos, char shade);
}
