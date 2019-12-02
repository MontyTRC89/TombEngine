#pragma once

#include "..\Global\global.h"

//#define Targetable ((int (__cdecl*)(ITEM_INFO*, AI_INFO*)) 0x004672F0) 
//#define TargetVisible ((int (__cdecl*)(ITEM_INFO*, AI_INFO*)) 0x004671E0)
//#define ShotLara ((int (__cdecl*)(ITEM_INFO*, AI_INFO*, BITE_INFO*, short, int)) 0x00467610)

int ShotLara(ITEM_INFO* item, AI_INFO* info, BITE_INFO* gun, short extra_rotation, int damage);
short GunMiss(int x, int y, int z, short speed, short yrot, short roomNumber);
short GunHit(int x, int y, int z, short speed, short yrot, short roomNumber);
short GunShot(int x, int y, int z, short speed, short yrot, short roomNumber);
int Targetable(ITEM_INFO* item, AI_INFO* info);
int TargetVisible(ITEM_INFO* item, AI_INFO* info);

void Inject_People();