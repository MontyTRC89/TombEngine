#pragma once

#include "..\Global\global.h"

#define InitialiseDoor ((void (__cdecl*)(short)) 0x0043DB60)
#define DrawLiftDoor ((void (__cdecl*)(ITEM_INFO*)) 0x0045AAF0)
#define DoubleDoorControl ((void (__cdecl*)(short)) 0x00429840)
#define InitialiseTrapDoor ((void (__cdecl*)(short)) 0x0043D2F0)
#define TrapDoorFloorCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x004891F0)
#define TrapDoorCeilingCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x00489450)
#define TrapDoorNormalCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x004896D0)
#define TrapDoorControl ((void (__cdecl*)(short)) 0x00488FA0)
#define InitialiseSteelDoor ((void (__cdecl*)(short)) 0x0043F180)
#define SteelDoorControl ((void (__cdecl*)(short)) 0x00486BE0)
#define SteelDoorCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x00487AD0)
void SequenceDoorControl(short itemNumber);
void UnderwaterDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void DoubleDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PushPullKickDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PushPullKickDoorControl(short itemNumber);
void DoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void DoorControl(short itemNumber);
void OpenThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
void ShutThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);

void Inject_Door();