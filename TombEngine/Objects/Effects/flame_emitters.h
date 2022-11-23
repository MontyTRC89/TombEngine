#pragma once

struct ItemInfo;
struct CollisionInfo;
struct InteractionBasis;

namespace TEN::Entities::Effects
{
	extern InteractionBasis FireBounds;

	void InitialiseFlameEmitter(short itemNumber);
	void InitialiseFlameEmitter2(short itemNumber);
	void InitialiseFlameEmitter3(short itemNumber);
	void FlameEmitterControl(short itemNumber);
	void FlameEmitter2Control(short itemNumber);
	void FlameEmitter3Control(short itemNumber);
	void FlameEmitterCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
