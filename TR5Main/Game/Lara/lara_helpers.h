#pragma once

bool TestLaraStep(COLL_INFO* coll);
bool TestLaraStepUp(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStepDown(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll);
bool IsStandingWeapon(LARA_WEAPON_TYPE gunType);
void SetLaraFallState(ITEM_INFO* item);
short GetLaraSlideDirection(COLL_INFO* coll);
void SetLaraSlideState(ITEM_INFO* item, COLL_INFO* coll);
