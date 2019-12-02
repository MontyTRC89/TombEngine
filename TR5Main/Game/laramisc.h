#pragma once
#include "..\Global\global.h"

void GetLaraDeadlyBounds();
void DelAlignLaraToRope(ITEM_INFO* item);
void InitialiseLaraAnims(ITEM_INFO* item);
void InitialiseLaraLoad(short itemNumber);
void InitialiseLara(int restore);
void LaraControl(short itemNumber);
void LaraCheat(ITEM_INFO* item, COLL_INFO* coll);
void LaraInitialiseMeshes();

//#define LaraBurn ((void (__cdecl*)()) 0x0048AD60)
//#define InitialiseLaraLoad ((void (__cdecl*)(short)) 0x004568C0)
