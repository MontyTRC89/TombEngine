#pragma once

#include "..\Global\global.h"

#define DoBloodSplat ((__int16 (__cdecl*)(__int32, __int32, __int32, __int16, __int16, __int16)) 0x00432760)
#define DoLotsOfBlood ((void (__cdecl*)(__int32, __int32, __int32, __int16, __int16, __int16, __int32)) 0x00432800)
#define CreateBubble ((__int16 (__cdecl*)(PHD_3DPOS*, __int16, __int32)) 0x0043C6C0); 
#define SetupRipple ((void (__cdecl*)(__int32, __int32, __int32, byte, byte)) 0x00430910);  

void __cdecl WadeSplash(ITEM_INFO* item, __int32 wh, __int32 wd);
void __cdecl Splash(ITEM_INFO* item);
void __cdecl SetupSplash(SPLASH_SETUP* setup);