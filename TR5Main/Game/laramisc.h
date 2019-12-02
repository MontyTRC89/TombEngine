#pragma once
#include "..\Global\global.h"

void __cdecl GetLaraDeadlyBounds();
void __cdecl DelAlignLaraToRope(ITEM_INFO* item);
void __cdecl InitialiseLaraAnims(ITEM_INFO* item);
void __cdecl InitialiseLaraLoad(short itemNumber);
void __cdecl InitialiseLara(int restore);
void __cdecl LaraControl(short itemNumber);
void __cdecl LaraCheat(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl LaraInitialiseMeshes();

//#define LaraBurn ((void (__cdecl*)()) 0x0048AD60)
//#define InitialiseLaraLoad ((void (__cdecl*)(short)) 0x004568C0)
