#pragma once

#include "..\Global\global.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

extern WINAPP	 App;
extern unsigned int threadId;
extern uintptr_t hThread;
extern HACCEL hAccTable;

#define GameClose ((__int32 (__cdecl*)(void)) 0x004A8575)
#define WinAppProc2 ((__int32 (__cdecl*)(HWND, UINT, WPARAM, LPARAM)) 0x004D2AB0)

__int32 __cdecl WinProcMsg();
__int32 __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, __int32 nShowCmd);
__int32 __cdecl WinClose();
LRESULT CALLBACK WinAppProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void __stdcall HandleWmCommand(unsigned __int16 wParam);

void Inject_WinMain();