#pragma once

#include "..\Global\global.h"

#define DoBloodSplat ((short (__cdecl*)(int, int, int, short, short, short)) 0x00432760)
#define DoLotsOfBlood ((void (__cdecl*)(int, int, int, short, short, short, int)) 0x00432800)
#define CreateBubble ((short (__cdecl*)(PHD_3DPOS*, short, int)) 0x0043C6C0); 
#define SetupRipple ((void (__cdecl*)(int, int, int, byte, byte)) 0x00430910);  

void __cdecl WadeSplash(ITEM_INFO* item, int wh, int wd);
void __cdecl Splash(ITEM_INFO* item);
void __cdecl SetupSplash(SPLASH_SETUP* setup);