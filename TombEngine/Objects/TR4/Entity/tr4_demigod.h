#pragma once

class Vector3i;
class Pose;

namespace TEN::Entities::TR4
{
	void InitialiseDemigod(short itemNumber);
	void DemigodControl(short itemNumber);
	void TriggerDemigodMissile(Pose* pose, short roomNumber, int flags);
	void DoDemigodEffects(short itemNumber);
	void TriggerHammerSmoke(int x, int y, int z, int maxSmokeCount);
	void TriggerDemigodMissileFlame(short fxNumber, short xVel, short yVel, short zVel);
}
