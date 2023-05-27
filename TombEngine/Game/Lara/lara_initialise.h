#pragma once

#include "Game/Lara/lara_struct.h"

void InitializeLara(bool restore);
void InitializeLaraMeshes(ItemInfo* item);
void InitializeLaraAnims(ItemInfo* item);
void InitializeLaraLoad(short itemNumber);
void InitializeLaraLevelJump(short itemNum, LaraInfo* lBackup);
void InitializeLaraDefaultInventory();