#pragma once
#pragma comment(linker,"/manifestdependency:\"" \
    "type='win32' " \
    "name='Microsoft.Windows.Common-Controls' " \
    "version='6.0.0.0' " \
    "processorArchitecture='*' "  \
    "publicKeyToken='6595b64144ccf1df' " \
    "language='*'\"")

typedef struct WINAPP
{
    HINSTANCE hInstance;
    int nFillMode;
    WNDCLASS WindowClass;
    HWND WindowHandle;
    bool bNoFocus;
    bool isInScene;
};

extern WINAPP App;
extern unsigned int ThreadID;
extern uintptr_t ThreadHandle;
extern HACCEL hAccTable;
extern HWND WindowsHandle;

// return handle
#define BeginThread(function, threadid) _beginthreadex(0, 0, &function, 0, 0, &threadid)
#define EndThread() _endthreadex(1)

int lua_exception_handler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description);
void WinProcMsg();
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
void WinClose();
LRESULT CALLBACK WinAppProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CALLBACK HandleWmCommand(unsigned short wParam);