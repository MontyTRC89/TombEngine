#pragma once

struct PHD_3DPOS;

namespace TEN::Entities::TR4
{
	void InitialiseDemigod(short itemNumber);
	void DemigodControl(short itemNumber);
	void TriggerDemigodMissile(PHD_3DPOS* pos, short roomNumber, int flags);
	void DoDemigodEffects(short itemNumber);
	void TriggerHammerSmoke(int x, int y, int z, int something);
	void TriggerDemigodMissileFlame(short fxNumber, short xVelocity, short yVelocity, short zVelocity);
}
