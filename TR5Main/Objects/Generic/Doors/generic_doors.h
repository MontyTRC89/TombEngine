#pragma once

struct DOORPOS_DATA;
struct DOOR_DATA;
struct ITEM_INFO;
struct COLL_INFO;
struct FLOOR_INFO;

namespace TEN::Entities::Doors
{
	void InitialiseDoor(short itemNumber);
	void DoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
	void DoorControl(short itemNumber);
	void OpenThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
	void ShutThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
}