#include "game.h"
#include "init.h"
#include "winmain.h"
#include <CommCtrl.h>

#include "..\resource.h"

#include <process.h>
#include <crtdbg.h>
#include <stdio.h>
#include <sol.hpp>

#include "..\Game\draw.h"
#include "..\Game\sound.h"
#include "..\Game\inventory.h"
#include "..\Game\control.h"
#include "..\Game\gameflow.h"
#include "..\Game\savegame.h"
#include "..\Specific\roomload.h"

#include "config.h"

WINAPP	 App;
unsigned int threadId;
uintptr_t hThread;
HACCEL hAccTable;
byte receivedWmClose = false;
bool Debug = false;

extern __int32 IsLevelLoading;
extern GameFlow* g_GameFlow;
extern GameScript* g_GameScript;
extern GameConfiguration g_Configuration;

__int32 __cdecl WinProcMsg()
{
	int result;
	struct tagMSG Msg;

	DB_Log(2, "WinProcMsg");
	do
	{
		GetMessageA(&Msg, 0, 0, 0);
		if (!TranslateAcceleratorA(WindowsHandle, hAccTable, &Msg))
		{
			TranslateMessage(&Msg);
			DispatchMessageA(&Msg);
		}
		result = Unk_876C48;
	} while (!Unk_876C48 && Msg.message != WM_QUIT);

	return result;
}

void __stdcall HandleWmCommand(unsigned __int16 wParam)
{
	if (wParam == 8)
	{
		DB_Log(5, "Pressed ALT + ENTER");

		if (!IsLevelLoading)
		{
			SuspendThread((HANDLE)hThread);
			DB_Log(5, "Game thread suspended");
			
			g_Renderer->ToggleFullScreen();

			ResumeThread((HANDLE)hThread);
			DB_Log(5, "Game thread resumed");

			if (g_Renderer->IsFullsScreen())
			{
				SetCursor(0);
				ShowCursor(false);
			}
			else
			{
				SetCursor(LoadCursorA(App.hInstance, (LPCSTR)0x68));
				ShowCursor(true);
			}
		}
	}
}

LRESULT CALLBACK WinAppProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Disables ALT + SPACE
	if (msg == WM_SYSCOMMAND && wParam == SC_KEYMENU) {
		return 0;
	}

	if (msg > 0x10)
	{
		if (msg == WM_COMMAND)
		{
			DB_Log(6, "WM_COMMAND");
			HandleWmCommand((unsigned __int16)wParam);
		}

		return DefWindowProcA(hWnd, msg, wParam, (LPARAM)lParam);
	}

	if (msg == WM_CLOSE)
	{
		DB_Log(6, "WM_CLOSE");

		receivedWmClose = true;
		PostQuitMessage(0);
		//DoTheGame = false;

		return DefWindowProcA(hWnd, 0x10u, wParam, (LPARAM)lParam);
	}

	if (msg != WM_ACTIVATE)
	{
		return DefWindowProcA(hWnd, msg, wParam, (LPARAM)lParam);
	}

	DB_Log(6, "WM_ACTIVATE");

	if (receivedWmClose)
	{
		return DefWindowProcA(hWnd, msg, wParam, (LPARAM)lParam);
	}

	if (App_Unk00D9AC2B)
		return 0;

	if ((__int16)wParam)
	{
		if ((signed __int32)(unsigned __int16)wParam > 0 && (signed __int32)(unsigned __int16)wParam <= 2)
		{
			DB_Log(6, "WM_ACTIVE");

			if (!Debug)
				ResumeThread((HANDLE)hThread);

			App_Unk00D9ABFD = 0;

			DB_Log(5, "Game Thread Resumed");

			return 0;
		}
	}
	else
	{
		DB_Log(6, "WM_INACTIVE");
		DB_Log(5, "HangGameThread");

		App_Unk00D9ABFD = 1;

		if (!Debug)
			SuspendThread((HANDLE)hThread);

		DB_Log(5, "Game Thread Suspended");
	}

	return 0;
}

__int32 __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, __int32 nShowCmd)
{
	int RetVal;
	int n;

	// Process the command line
	bool setup = false;
	
	LPWSTR* argv;
	__int32 argc;
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	for (__int32 i = 1; i < argc; i++)
	{
		if (wcscmp(argv[i], L"/setup") == 0)
			setup = true;
		if (wcscmp(argv[i], L"/debug") == 0)
			Debug = true;
	}

	LocalFree(argv);

	// Clear Application Structure
	memset(&App, 0, sizeof(WINAPP));

	_CrtSetReportMode(0, 2);
	_CrtSetDbgFlag(-1);
	 
	// TODO: deprecated
	//LoadGameflow();
	//LoadSettings();

	// Initialise the new scripting system
	sol::state luaState;
	luaState.open_libraries(sol::lib::base);

	g_GameFlow = new GameFlow(&luaState);
	LoadScript();

	g_GameScript = new GameScript(&luaState);

	// Initialise chunks for savegames
	SaveGame::Start();

	// Initialise main window
	App.hInstance = hInstance;
	App.WindowClass.hIcon = NULL;
	App.WindowClass.lpszMenuName = NULL;
	App.WindowClass.lpszClassName = "TR5Main";
	App.WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	App.WindowClass.hInstance = hInstance;
	App.WindowClass.style = CS_VREDRAW | CS_HREDRAW;
	App.WindowClass.lpfnWndProc = (WNDPROC)WinAppProc;
	App.WindowClass.cbClsExtra = 0;
	App.WindowClass.cbWndExtra = 0;
	App.WindowClass.hCursor = LoadCursor(App.hInstance, IDC_ARROW);

	if (!RegisterClass(&App.WindowClass))
	{
		printf("Unable To Register Window Class\n");
		return FALSE;
	}

	// Create the renderer and enumerate adapters and video modes
	g_Renderer = new Renderer11();
	g_Renderer->Create();
	g_Renderer->EnumerateVideoModes();

	// Load configuration and optionally show the setup dialog
	if (setup || !LoadConfiguration())
	{
		if (!SetupDialog())
		{
			WinClose();
			return 0;
		}

		LoadConfiguration();
	}

	tagRECT Rect;

	Rect.left = 0;
	Rect.top = 0;
	Rect.right = g_Configuration.Width;
	Rect.bottom = g_Configuration.Height;

	AdjustWindowRect(&Rect, WS_CAPTION, false);

	App.WindowHandle = CreateWindowEx(
		0,
		"TR5Main",
		g_GameFlow->GetSettings()->WindowTitle.c_str(),
		WS_POPUP,
		0,
		0,
		Rect.right - Rect.left,
		Rect.bottom - Rect.top,
		NULL,
		NULL,
		App.hInstance,
		NULL
	);

	if (!App.WindowHandle)
	{
		printf("Unable To Create Window: %d\n", GetLastError());
		return FALSE;
	}

	PhdWidth = g_Configuration.Width;
	PhdHeight = g_Configuration.Height;
	WindowsHandle = App.WindowHandle;

	// Initialise the renderer
	g_Renderer->Initialise(g_Configuration.Width, g_Configuration.Height, g_Configuration.RefreshRate, 
						   g_Configuration.Windowed, App.WindowHandle);

	// Initialize audio
	if (g_Configuration.EnableSound)	
		Sound_Init();

	// Initialise the new inventory
	g_Inventory = new Inventory();

	
	App.bNoFocus = false;
	App.isInScene = false;

	UpdateWindow(WindowsHandle);
	ShowWindow(WindowsHandle, nShowCmd);

	SetCursor(0);
	ShowCursor(0);
	hAccTable = LoadAcceleratorsA(hInstance, (LPCSTR)0x65);

	//g_Renderer->Test();

	SoundActive = false;
	DoTheGame = true;

	Unk_876C48 = false;
	hThread = _beginthreadex(0, 0, &GameMain, 0, 0, &threadId); 
	WinProcMsg();
	Unk_876C48 = true;

	while (DoTheGame);
	
	WinClose();

	return 0;
}

__int32 __cdecl WinClose()
{
	DB_Log(2, "WinClose - DLL");

	DestroyAcceleratorTable(hAccTable);

	if (g_Configuration.EnableSound)
		Sound_DeInit();
	
	delete g_Renderer;
	delete g_Inventory;
	delete g_GameFlow;

	SaveGame::End();

	return 0;
}


void Inject_WinMain()
{
	INJECT(0x004D23E0, WinClose);
	INJECT(0x004D24C0, WinProcMsg);
	INJECT(0x004D1C00, WinMain);
}