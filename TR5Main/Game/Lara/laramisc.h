#pragma once
#include "collide.h"

extern COLL_INFO lara_coll;


void GetLaraDeadlyBounds();
void DelAlignLaraToRope(ITEM_INFO* item);
void InitialiseLaraAnims(ITEM_INFO* item);
void InitialiseLaraLoad(short itemNumber);
void InitialiseLara(int restore);
void LaraInitialiseMeshes();
void AnimateLara(ITEM_INFO* item);
void LaraCheatyBits();
void LaraCheatGetStuff();
void DelsGiveLaraItemsCheat();
