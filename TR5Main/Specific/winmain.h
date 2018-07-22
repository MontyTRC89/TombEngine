#pragma once

#include "..\Global\global.h"

extern WINAPP	 App;
extern unsigned int threadId;
extern uintptr_t hThread;
extern HACCEL hAccTable;

#define TIME_Init ((void (__cdecl*)(void)) 0x004D19D0)
#define GameClose ((__int32 (__cdecl*)(void)) 0x004A8575)
#define SOUND_Init ((__int32 (__cdecl*)(void)) 0x004790A0)
#define LoadGameflow ((__int32 (__cdecl*)(void)) 0x00434800)
#define WinAppProc ((__int32 (__cdecl*)(HWND, UINT, WPARAM, LPARAM)) 0x004D2AB0)

__int32 __cdecl WinProcMsg();
__int32 __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, __int32 nShowCmd);
__int32 __cdecl WinClose();

void Inject_WinMain();