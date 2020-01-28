#pragma once
#include "..\Global\global.h"

extern COLL_INFO coll;

#define GetLaraDeadlyBounds ((void (__cdecl*)()) 0x004569C0)
#define phd_SetTrans ((void (__cdecl*)(int, int, int)) 0x0048FA40)
#define phd_RotBoundingBoxNoPersp ((void (__cdecl*)(short*, short*)) 0x0042E240)

//void GetLaraDeadlyBounds();
void DelAlignLaraToRope(ITEM_INFO* item);
void InitialiseLaraAnims(ITEM_INFO* item);
void InitialiseLaraLoad(short itemNumber);
void InitialiseLara(int restore);
void LaraControl(short itemNumber);
void LaraCheat(ITEM_INFO* item, COLL_INFO* coll);
void LaraInitialiseMeshes();
void AnimateLara(ITEM_INFO* item);
void LaraCheatyBits();