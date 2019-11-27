#pragma once

#include "..\Global\global.h"

#define DoBloodSplat ((short (__cdecl*)(int, int, int, short, short, short)) 0x00432760)
#define DoLotsOfBlood ((void (__cdecl*)(int, int, int, short, short, short, int)) 0x00432800)
#define CreateBubble ((short (__cdecl*)(PHD_3DPOS*, short, int)) 0x0043C6C0); // 0x00483350)

