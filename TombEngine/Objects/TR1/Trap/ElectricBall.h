#pragma once

struct CollisionInfo;
struct ItemInfo;
struct ObjectInfo;

namespace TEN::Entities::Traps
{
	void InitializeElectricBall(short itemNumber);

	void ControlElectricBall(short itemNumber);
	void InitializeElectricBallImpactPoint(short itemNumber);
}
