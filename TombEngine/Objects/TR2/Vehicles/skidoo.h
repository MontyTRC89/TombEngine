#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

namespace TEN::Entities::Vehicles
{
	void InitialiseSkidoo(short itemNumber);

	int GetSkidooMountType(ItemInfo* laraItem, ItemInfo* skidooItem, CollisionInfo* coll);
	bool TestSkidooDismountOK(ItemInfo* skidooItem, int direction);
	bool TestSkidooDismount(ItemInfo* laraItem, ItemInfo* skidooItem);

	int GetSkidooCollisionAnim(ItemInfo* skidooItem, Vector3Int* moved);
	void SkidooCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	void SkidooGuns(ItemInfo* laraItem, ItemInfo* skidooItem);
	void DoSnowEffect(ItemInfo* skidooItem);

	bool SkidooControl(ItemInfo* laraItem, CollisionInfo* coll);
	bool SkidooUserControl(ItemInfo* laraItem, ItemInfo* skidooItem, int height, int* pitch);
	void SkidooAnimation(ItemInfo* laraItem, ItemInfo* skidooItem, int collide, bool dead);

	int SkidooDynamics(ItemInfo* laraItem, ItemInfo* skidooItem);
	int TestSkidooHeight(ItemInfo* skidooItem, int zOffset, int xOffset, Vector3Int* pos);
	short DoSkidooShift(ItemInfo* skidooItem, Vector3Int* pos, Vector3Int* old);
	int DoSkidooDynamics(int height, int verticalVelocity, int* y);

	void DrawSkidoo(ItemInfo* skidooItem);
}
