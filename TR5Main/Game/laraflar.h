#pragma once

#define UndrawFlaresMeshes ((void (__cdecl*)()) 0x004553E0)
#define CreateFlare ((void (__cdecl*)(__int16, __int32)) 0x00454BC0)
#define DrawFlare ((void (__cdecl*)()) 0x00454F50)
#define UndrawFlare ((void (__cdecl*)()) 0x004550C0)  
#define DoFlareInHand ((void (__cdecl*)(__int32)) 0x004549B0)  
#define SetFlareArm ((void (__cdecl*)(__int32)) 0x00454EE0)  
// #define DrawFlaresMeshes ((void (__cdecl*)()) 0x004553B0)

void __cdecl DrawFlareMeshes();

void Inject_LaraFlar();
