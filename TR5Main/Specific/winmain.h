#pragma once

#include "..\Global\global.h"

extern WINAPP	 App;
extern unsigned int threadId;
extern uintptr_t hThread;
extern HACCEL hAccTable;

#define GameClose ((__int32 (__cdecl*)(void)) 0x004A8575)
#define WinAppProc ((__int32 (__cdecl*)(HWND, UINT, WPARAM, LPARAM)) 0x004D2AB0)

__int32 __cdecl WinProcMsg();
__int32 __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, __int32 nShowCmd);
__int32 __cdecl WinClose();

void Inject_WinMain();