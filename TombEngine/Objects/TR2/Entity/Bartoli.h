#pragma once

enum GAME_OBJECT_ID : short;
struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR2
{
	void InitializeBartoli(short itemNumber);
	void ControlBartoli(short itemNumber);

	void SpawnDragonExplosion(const ItemInfo& originItem, GAME_OBJECT_ID objectID);
	void ControlDragonExplosion(int itemNumber);
}
