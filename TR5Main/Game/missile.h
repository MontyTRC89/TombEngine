#pragma once

#include "..\Global\global.h"

void ShootAtLara(FX_INFO *fx);
void ControlMissile(short fxNumber);
#define TorpedoControl ((void (__cdecl*)(short)) 0x0045C9F0)
#define ControlEnemyMissile ((void (__cdecl*)(short)) 0x00431E70)