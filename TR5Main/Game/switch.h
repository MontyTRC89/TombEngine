#pragma once
#include "..\Global\global.h"

#define InitialiseSwitch ((void (__cdecl*)(__int16)) 0x00440070)
#define SequenceControl ((void (__cdecl*)(__int16)) 0x0047F520)
#define SequenceCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x0047F610)
#define LeverSwitchCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x0047EE00)
#define InitialisePulleySwitch ((void (__cdecl*)(__int16)) 0x0043E1F0)
#define InitialiseCrowDoveSwitch ((void (__cdecl*)(__int16)) 0x0043ECF0)
void __cdecl CrowDoveSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl CrowDoveSwitchControl(__int16 itemNumber);
void __cdecl CogSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl CogSwitchControl(__int16 itemNum);
void __cdecl FullBlockSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl FullBlockSwitchControl(__int16 itemNumber);
void __cdecl CrowbarSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl JumpSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl RailSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl TurnSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl TurnSwitchControl(__int16 itemNum);
void __cdecl PulleyCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl UnderwaterSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl SwitchCollision2(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl SwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl SwitchControl(__int16 itemNumber);
void __cdecl TestTriggersAtXYZ(__int32 x, __int32 y, __int32 z, __int16 roomNumber, __int32 heavy, __int32 flags);
__int32 __cdecl GetKeyTrigger(ITEM_INFO* item);
__int32 __cdecl GetSwitchTrigger(ITEM_INFO* item, __int16* itemNos, __int32 AttatchedToSwitch);
__int32 __cdecl SwitchTrigger(__int16 itemNum, __int16 timer);

void Inject_Switch();