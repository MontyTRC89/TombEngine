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
#define InitialiseCupboard ((void (__cdecl*)(short)) 0x0043EDB0)
#define CupboardCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x004699A0)
#define CupboardControl ((void (__cdecl*)(short)) 0x00469660)
#define InitialiseSteelDoor ((void (__cdecl*)(short)) 0x0043F180)
#define SteelDoorControl ((void (__cdecl*)(short)) 0x00486BE0)
#define SteelDoorCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x00487AD0)
void __cdecl SequenceDoorControl(short itemNumber);
void __cdecl UnderwaterDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl DoubleDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl PushPullKickDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl PushPullKickDoorControl(short itemNumber);
void __cdecl DoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl DoorControl(short itemNumber);
void __cdecl OpenThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);
void __cdecl ShutThatDoor(DOORPOS_DATA* doorPos, DOOR_DATA* dd);

void Inject_Door();