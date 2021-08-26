#pragma once
#include "effect2.h"

void ShootAtLara(FX_INFO* fx);
void ControlMissile(short fxNumber);
void ControlNatlaGun(short fx_number);

short ShardGun(int x, int y, int z, short speed, short yrot, short roomNumber);
short BombGun(int x, int y, int z, short speed, short yrot, short roomNumber); // RocketGun = BombGun
short NatlaGun(int x, int y, int z, short speed, short yrot, short roomNumber);