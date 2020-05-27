#pragma once

#include "global.h"

#pragma comment(linker,"/manifestdependency:\"" \
    "type='win32' " \
    "name='Microsoft.Windows.Common-Controls' " \
    "version='6.0.0.0' " \
    "processorArchitecture='*' "  \
    "publicKeyToken='6595b64144ccf1df' " \
    "language='*'\"")

extern WINAPP	 App;
extern unsigned int ThreadID;
extern uintptr_t ThreadHandle;
extern HACCEL hAccTable;
extern HWND WindowsHandle;

int lua_exception_handler(lua_State *L, sol::optional<const exception&> maybe_exception, sol::string_view description);
void WinProcMsg();
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
void WinClose();
LRESULT CALLBACK WinAppProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CALLBACK HandleWmCommand(unsigned short wParam);