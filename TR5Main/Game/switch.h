#pragma once

#include "collide.h"

extern int PulleyItemNumber;

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
void ShootSwitchCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
