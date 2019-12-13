#pragma once

//#define InitialiseHair ((void (__cdecl*)()) 0x00438BE0)
#define HairControl ((short (__cdecl*)(short, short, short)) 0x00438C80)

void InitialiseHair();

void Inject_Hair();