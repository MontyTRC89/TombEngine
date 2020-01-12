#pragma once
#include "..\Global\global.h"

//#define InitialiseSwitch ((void (__cdecl*)(short)) 0x00440070)
//#define SequenceControl ((void (__cdecl*)(short)) 0x0047F520)
//#define SequenceCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x0047F610)
//#define LeverSwitchCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x0047EE00)
//#define InitialisePulleySwitch ((void (__cdecl*)(short)) 0x0043E1F0)
//#define InitialiseCrowDoveSwitch ((void (__cdecl*)(short)) 0x0043ECF0)
void CrowDoveSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void CrowDoveSwitchControl(short itemNumber);
void CogSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void CogSwitchControl(short itemNum);
void FullBlockSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void FullBlockSwitchControl(short itemNumber);
void CrowbarSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void JumpSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void RailSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void TurnSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void TurnSwitchControl(short itemNum);
void PulleyCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void UnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void SwitchCollision2(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void SwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void SwitchControl(short itemNumber);
void TestTriggersAtXYZ(int x, int y, int z, short roomNumber, int heavy, int flags);
int GetKeyTrigger(ITEM_INFO* item);
int GetSwitchTrigger(ITEM_INFO* item, short* itemNos, int AttatchedToSwitch);
int SwitchTrigger(short itemNum, short timer);
void InitialiseSwitch(short itemNum);
void InitialisePulleySwitch(short itemNumber);
void InitialiseCrowDoveSwitch(short itemNumber);
void ProcessExplodingSwitchType8(ITEM_INFO* item);
void InitialiseShootSwitch(short itemNumber);

extern int PulleyItemNumber;

void Inject_Switch();