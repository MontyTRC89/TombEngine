#pragma once

#include "..\Global\global.h"

void ShootAtLara(FX_INFO *fx);
void ControlMissile(__int16 fxNumber);
#define TorpedoControl ((void (__cdecl*)(__int16)) 0x0045C9F0)
#define ControlEnemyMissile ((void (__cdecl*)(__int16)) 0x00431E70)