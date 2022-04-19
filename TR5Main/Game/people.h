#pragma once
#include "Game/control/box.h"

constexpr auto MAX_VISIBILITY_DISTANCE = SECTOR(8);

bool ShotLara(ITEM_INFO* item, AI_INFO* AI, BITE_INFO* bite, short extraRotation, int damage);
short GunMiss(int x, int y, int z, short velocity, short yRot, short roomNumber);
short GunHit(int x, int y, int z, short velocity, short yRot, short roomNumber);
short GunShot(int x, int y, int z, short velocity, short yRot, short roomNumber);
bool Targetable(ITEM_INFO* item, AI_INFO* AI);
bool TargetVisible(ITEM_INFO* item, AI_INFO* AI);
