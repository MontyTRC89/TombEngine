#pragma once
#include "Game/collision/collide_room.h"

namespace TEN::Entities::TR4
{
	void InitialiseSas(short itemNumber);
	void SasControl(short itemNumber);
	void InitialiseInjuredSas(short itemNumber);
	void InjuredSasControl(short itemNumber);
	void SasDragBlokeCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
	void SasFireGrenade(ITEM_INFO* item, short angle1, short angle2);
}