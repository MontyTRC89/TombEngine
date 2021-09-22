#pragma once

struct ITEM_INFO;
struct COLL_INFO;
struct OBJECT_COLLISION_BOUNDS;

namespace TEN::Entities::Effects
{
	extern OBJECT_COLLISION_BOUNDS FireBounds;

	void FlameEmitterControl(short itemNumber);
	void FlameEmitter2Control(short itemNumber);
	void FlameControl(short fxNumber);
	void InitialiseFlameEmitter(short itemNumber);
	void InitialiseFlameEmitter2(short itemNumber);
	void FlameEmitter3Control(short itemNumber);
	void FlameEmitterCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
}