#pragma once

#define InitialiseHair ((void (__cdecl*)()) 0x00438BE0)
#define HairControl ((__int16 (__cdecl*)(__int16, __int16, __int16)) 0x00438C80)

void __cdecl j_HairControl(__int16 a, __int16 b, __int16 c);

void Inject_Hair();