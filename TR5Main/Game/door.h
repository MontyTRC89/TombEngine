#pragma once
#include "items.h"
#include "collide.h"
#include "room.h"

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

void SequenceDoorControl(short itemNumber);
void UnderwaterDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void DoubleDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PushPullKickDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PushPullKickDoorControl(short itemNumber);
void DoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void DoorControl(short itemNumber);
void OpenThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
void ShutThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
void InitialiseDoor(short itemNumber);
void InitialiseSteelDoor(short itemNumber);
void SteelDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void AssignClosedDoor(ITEM_INFO* item);
void InitialiseClosedDoors();
void ProcessClosedDoors();
void GetClosedDoorNormal(ROOM_INFO* room, short** dptr, byte* n, int z, int x, int absX, int absZ);
void FillDoorPointers(DOOR_DATA* doorData, ITEM_INFO* item, short roomNumber, int dz, int dx);
