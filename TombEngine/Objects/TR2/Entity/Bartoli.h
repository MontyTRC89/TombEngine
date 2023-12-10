#pragma once

enum GAME_OBJECT_ID : short;
struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR2
{
	void InitializeBartoli(short itemNumber);
	void ControlBartoli(short itemNumber);

	void ControlBartoliTransformEffect(int itemNumber);
}
