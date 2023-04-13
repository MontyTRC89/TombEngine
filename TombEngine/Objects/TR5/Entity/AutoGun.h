#pragma once

class Vector3i;
struct ObjectInfo;

namespace TEN::Entities::Creatures::TR5
{
	void InitializeAutoGuns(short itemNumber);
	void ControlAutoGun(short itemNumber);
	void SpawnAutoGunSmoke(const Vector3& pos, char shade);
}
