#pragma once
#include "Math/Math.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	void TriggerTorchFlame(char fxObject, char node);
	void DoFlameTorch();
	void GetFlameTorch();
	void TorchControl(short itemNumber);
	void LaraTorch(Vector3i* origin, Vector3i* target, int rot, int color);
	void FireCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
