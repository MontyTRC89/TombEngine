#pragma once
#include "Game/Lara/lara_struct.h"

void InitialiseLara(bool restore);
void InitialiseLaraMeshes(ItemInfo* item);
void InitialiseLaraAnims(ItemInfo* item);
void InitialiseLaraLoad(short itemNumber);
void InitialiseLaraLevelJump(short itemNum, LaraInfo* lBackup);
void InitialiseLaraDefaultInventory();