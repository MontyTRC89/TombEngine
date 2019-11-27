#pragma once
#include "..\Global\global.h"

//#define InitialiseSwitch ((void (__cdecl*)(short)) 0x00440070)
//#define SequenceControl ((void (__cdecl*)(short)) 0x0047F520)
//#define SequenceCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x0047F610)
//#define LeverSwitchCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x0047EE00)
//#define InitialisePulleySwitch ((void (__cdecl*)(short)) 0x0043E1F0)
//#define InitialiseCrowDoveSwitch ((void (__cdecl*)(short)) 0x0043ECF0)
void __cdecl CrowDoveSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl CrowDoveSwitchControl(short itemNumber);
void __cdecl CogSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl CogSwitchControl(short itemNum);
void __cdecl FullBlockSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl FullBlockSwitchControl(short itemNumber);
void __cdecl CrowbarSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl JumpSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl RailSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl TurnSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl TurnSwitchControl(short itemNum);
void __cdecl PulleyCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl UnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl SwitchCollision2(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl SwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl SwitchControl(short itemNumber);
void __cdecl TestTriggersAtXYZ(int x, int y, int z, short roomNumber, int heavy, int flags);
int __cdecl GetKeyTrigger(ITEM_INFO* item);
int __cdecl GetSwitchTrigger(ITEM_INFO* item, short* itemNos, int AttatchedToSwitch);
int __cdecl SwitchTrigger(short itemNum, short timer);
void __cdecl InitialiseSwitch(short itemNum);
void __cdecl InitialisePulleySwitch(short itemNumber);
void __cdecl InitialiseCrowDoveSwitch(short itemNumber);

void Inject_Switch();