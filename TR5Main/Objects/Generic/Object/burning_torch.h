#pragma once
#include "Specific/phd_global.h"

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::Generic
{
	void TriggerTorchFlame(char fxObj, char node);
	void DoFlameTorch();
	void GetFlameTorch();
	void TorchControl(short itemNumber);
	void LaraTorch(Vector3Int* src, Vector3Int* target, int rot, int color);
	void FireCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
