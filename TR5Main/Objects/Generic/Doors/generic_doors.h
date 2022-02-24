#pragma once

struct ITEM_INFO;
struct COLL_INFO;
struct ROOM_INFO;
struct DOORPOS_DATA;
struct DOOR_DATA;

namespace TEN::Entities::Doors
{
	void InitialiseDoor(short itemNumber);
	void DoorCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
	void DoorControl(short itemNumber);
	void OpenThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
	void ShutThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
}
