#pragma once
#include "Game/effects/effects.h"

struct ItemInfo;

void ShootAtLara(ItemInfo& fx);
void ControlMissile(short fxNumber);
void ControlNatlaGun(short fxNumber);

short ShardGun(int x, int y, int z, short velocity, short yRot, short roomNumber);
short BombGun(int x, int y, int z, short velocity, short yRot, short roomNumber);
