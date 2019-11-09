#pragma once
#include "..\Global\global.h"

void __cdecl SequenceDoorControl(__int16 itemNumber);
void __cdecl UnderwaterDoorCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl DoubleDoorCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl PushPullKickDoorCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl PushPullKickDoorControl(__int16 itemNumber);
void __cdecl DoorCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl DoorControl(__int16 itemNumber);
void __cdecl OpenThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
void __cdecl ShutThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);

void Inject_Door();