#pragma once

class Vector3i;

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseLaserHead(short itemNumber);
	void LaserHeadControl(short itemNumber);

	void SpawnLaserHeadSparks(const Vector3& pos, const Vector3& color, int count, int unk = 0);
}
