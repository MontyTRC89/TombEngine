#pragma once

#include "..\Global\global.h"

#pragma comment(linker,"/manifestdependency:\"" \
    "type='win32' " \
    "name='Microsoft.Windows.Common-Controls' " \
    "version='6.0.0.0' " \
    "processorArchitecture='*' "  \
    "publicKeyToken='6595b64144ccf1df' " \
    "language='*'\"")

extern WINAPP	 App;
extern unsigned int threadId;
extern uintptr_t hThread;
extern HACCEL hAccTable;

#define GameClose ((int (__cdecl*)(void)) 0x004A8575)
#define WinAppProc2 ((int (__cdecl*)(HWND, UINT, WPARAM, LPARAM)) 0x004D2AB0)

int WinProcMsg();
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
int WinClose();
LRESULT CALLBACK WinAppProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void __stdcall HandleWmCommand(unsigned short wParam);

void Inject_WinMain();