#pragma once

struct CollisionInfo;
struct ItemInfo;

struct CollisionInfo;
struct ItemInfo;
struct ObjectInfo;

namespace TEN::Entities::TR4
{
	void SetupSas(ObjectInfo& object);
	void SetupInjuredSas(ObjectInfo& object);
	void SetupSasDraggableSas(ObjectInfo& object);

	void InitialiseSas(short itemNumber);
	void InitialiseInjuredSas(short itemNumber);

	void SasControl(short itemNumber);
	void InjuredSasControl(short itemNumber);

	void SasDragBlokeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void SasFireGrenade(ItemInfo& item, short angle1, short angle2);
}
