#pragma once

struct ItemInfo;
struct CollisionInfo;
struct ROOM_INFO;
struct DOORPOS_DATA;
struct DOOR_DATA;

namespace TEN::Entities::Doors
{
	void InitialiseDoor(short itemNumber);
	void DoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoorControl(short itemNumber);
	void OpenThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
	void ShutThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
}
