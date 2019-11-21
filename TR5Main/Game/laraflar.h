#pragma once
#include "..\Global\global.h"

/*#define UndrawFlaresMeshes ((void (__cdecl*)()) 0x004553E0)
#define CreateFlare ((void (__cdecl*)(__int16, __int32)) 0x00454BC0)
#define DrawFlare ((void (__cdecl*)()) 0x00454F50)
#define UndrawFlare ((void (__cdecl*)()) 0x004550C0)  
#define DoFlareInHand ((void (__cdecl*)(__int32)) 0x004549B0)  
#define SetFlareArm ((void (__cdecl*)(__int32)) 0x00454EE0)  
#define FlareControl ((void (__cdecl*)(__int16)) 0x00455460)
#define DrawFlareItem ((void (__cdecl*)(ITEM_INFO*)) 0x00454A90)*/

extern void FlareControl(short item_number);
extern void ready_flare();
extern void undraw_flare_meshes();
extern void draw_flare_meshes();
extern void undraw_flare();
extern void draw_flare();
extern void set_flare_arm(int frame);
extern void CreateFlare(short object, int thrown);
extern void DrawFlareInAir(struct ITEM_INFO* item);
extern void DoFlareInHand(int flare_age);
extern int DoFlareLight(struct PHD_VECTOR* pos, int flare_age);

//void __cdecl DrawFlareMeshes();

void Inject_LaraFlar();