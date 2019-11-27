#pragma once

#define InitialiseHair ((void (__cdecl*)()) 0x00438BE0)
#define HairControl ((short (__cdecl*)(short, short, short)) 0x00438C80)

void __cdecl j_HairControl(short a, short b, short c);

void Inject_Hair();