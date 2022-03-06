#pragma once
#include "Game/control/box.h"

constexpr auto MAX_VISIBILITY_DISTANCE = WALL_SIZE * 8;

bool ShotLara(ITEM_INFO* item, AI_INFO* info, BITE_INFO* gun, short extra_rotation, int damage);
short GunMiss(int x, int y, int z, short speed, short yrot, short roomNumber);
short GunHit(int x, int y, int z, short speed, short yrot, short roomNumber);
short GunShot(int x, int y, int z, short speed, short yrot, short roomNumber);
bool Targetable(ITEM_INFO* item, AI_INFO* info);
bool TargetVisible(ITEM_INFO* item, AI_INFO* info);
