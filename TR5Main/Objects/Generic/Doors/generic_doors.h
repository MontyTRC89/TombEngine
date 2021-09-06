#pragma once
#include "items.h"
#include "collide.h"
#include "room.h"

namespace TEN::Entities::Doors
{
	struct DOORPOS_DATA
	{
		FLOOR_INFO* floor;
		FLOOR_INFO data;
		short block;
	};

	struct DOOR_DATA
	{
		DOORPOS_DATA d1;
		DOORPOS_DATA d1flip;
		DOORPOS_DATA d2;
		DOORPOS_DATA d2flip;
		short opened;
		short* dptr1;
		short* dptr2;
		short* dptr3;
		short* dptr4;
		byte dn1;
		byte dn2;
		byte dn3;
		byte dn4;
		ITEM_INFO* item;
	};

	void InitialiseDoor(short itemNumber);
	void DoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
	void DoorControl(short itemNumber);
	void OpenThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
	void ShutThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
}