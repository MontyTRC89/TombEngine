#pragma once

void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraCrawlVault(ITEM_INFO* item, COLL_INFO* coll);
void SetLaraFallState(ITEM_INFO* item);
void SetLaraFallBackState(ITEM_INFO* item);
short GetLaraSlideDirection(COLL_INFO* coll);
void SetLaraSlideState(ITEM_INFO* item, COLL_INFO* coll);
