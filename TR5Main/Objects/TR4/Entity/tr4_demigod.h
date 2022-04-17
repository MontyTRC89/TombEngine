#pragma once

class PoseData;

namespace TEN::Entities::TR4
{
	void InitialiseDemigod(short itemNumber);
	void DemigodControl(short itemNumber);
	void TriggerDemigodMissile(PoseData* pos, short roomNumber, int flags);
	void DoDemigodEffects(short itemNumber);
	void TriggerHammerSmoke(int x, int y, int z, int something);
	void TriggerDemigodMissileFlame(short fxNumber, short xVelocity, short yVelocity, short zVelocity);
}
