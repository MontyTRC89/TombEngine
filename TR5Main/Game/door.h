#pragma once

#include "..\Global\global.h"

//#define InitialiseDoor ((void (__cdecl*)(short)) 0x0043DB60)
#define DrawLiftDoor ((void (__cdecl*)(ITEM_INFO*)) 0x0045AAF0)
#define DoubleDoorControl ((void (__cdecl*)(short)) 0x00429840)
//#define InitialiseSteelDoor ((void (__cdecl*)(short)) 0x0043F180)
#define SteelDoorControl ((void (__cdecl*)(short)) 0x00486BE0)
//#define SteelDoorCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x00487AD0)
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

void Inject_Door();