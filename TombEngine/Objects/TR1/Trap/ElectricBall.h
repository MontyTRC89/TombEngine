#pragma once

struct CollisionInfo;
struct ItemInfo;
struct ObjectInfo;

namespace TEN::Entities::Traps
{
	void InitializeElectricBall(short itemNumber);
	;
	void ControlElectricBall(short itemNumber);
	void InitializeElectricBallImpactPoint(short itemNumber);
	static void TriggerElectricBallShockwaveAttackSparks(int x, int y, int z, byte r, byte g, byte b, byte size);
}
