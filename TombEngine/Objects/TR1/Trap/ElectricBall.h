#pragma once

struct ItemInfo;
class Vector3i;
struct CreatureBiteInfo;

namespace TEN::Entities::Traps
{
	void InitializeElectricBall(short itemNumber);
	void SpawnElectricBallLightning(ItemInfo& item, const Vector3& pos, const CreatureBiteInfo& bite);
	void ControlElectricBall(short itemNumber);
	void InitializeElectricBallImpactPoint(short itemNumber);
	void TriggerElectricBallShockwaveAttackSparks(int x, int y, int z, byte r, byte g, byte b, byte size);	
}
