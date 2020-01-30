#pragma once

#include "..\Global\global.h"

//#define TorpedoControl ((void (__cdecl*)(short)) 0x0045C9F0)
#define ControlEnemyMissile ((void (__cdecl*)(short)) 0x00431E70)

void ShootAtLara(FX_INFO* fx);
void ControlMissile(short fxNumber);
void ControlNatlaGun(short fx_number);

short ShardGun(int x, int y, int z, short speed, short yrot, short room_number);
short BombGun(int x, int y, int z, short speed, short yrot, short room_number); // RocketGun = BombGun
short NatlaGun(int x, int y, int z, short speed, short yrot, short room_number);