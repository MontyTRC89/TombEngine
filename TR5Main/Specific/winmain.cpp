#include "init.h"
#include "winmain.h"
#include <CommCtrl.h>

#include "..\resource.h"

#include <process.h>
#include <crtdbg.h>
#include <stdio.h>
#include "sol.hpp"

#include "..\Game\draw.h"
#include "..\Game\sound.h"
#include "..\Game\inventory.h"
#include "..\Game\control.h"
#include "..\Game\gameflow.h"
#include "..\Game\savegame.h"
#include "..\Specific\level.h"
#include "..\Specific\level.h"

#include "configuration.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
WINAPP	 App;
unsigned int threadId;
uintptr_t hThread;
HACCEL hAccTable;
byte receivedWmClose = false;
bool Debug = false;
HWND WindowsHandle;
int App_Unk00D9ABFD;
extern int IsLevelLoading;
extern GameFlow* g_GameFlow;
extern GameScript* g_GameScript;
extern GameConfiguration g_Configuration;
DWORD DebugConsoleThreadID;
DWORD MainThreadID;
bool BlockAllInput = true;

int lua_exception_handler(lua_State* L, sol::optional<const exception&> maybe_exception, sol::string_view description)
{
	return luaL_error(L, description.data());
}

int WinProcMsg()
{
	int result;
	struct tagMSG Msg;

	//DB_Log(2, "WinProcMsg");
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

void __stdcall HandleWmCommand(unsigned short wParam)
{
	if (wParam == 8)
	{
		//DB_Log(5, "Pressed ALT + ENTER");

		if (!IsLevelLoading)
		{
			SuspendThread((HANDLE)hThread);
			//DB_Log(5, "Game thread suspended");
			
			g_Renderer->ToggleFullScreen();

			ResumeThread((HANDLE)hThread);
			//DB_Log(5, "Game thread resumed");

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

void HandleScriptMessage(WPARAM wParam)
{
	string ErrorMessage;
	string message = *(string*)(wParam);
	bool status;

	//check whether line starts with "lua "
	if (message.find("lua ") == 0) {
		string scriptSubstring = message.substr(4);
		status = g_GameScript->ExecuteScript(scriptSubstring, ErrorMessage);
	}
	else {
		status = g_GameScript->ExecuteString(message, ErrorMessage);
	}
	if (!status)
		cout << ErrorMessage << endl;
}

LRESULT CALLBACK WinAppProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_USER + 0) {
		HandleScriptMessage(wParam);

		return 0;
	}
	// Disables ALT + SPACE
	if (msg == WM_SYSCOMMAND && wParam == SC_KEYMENU) {
		return 0;
	}

	if (msg > 0x10)
	{
		if (msg == WM_COMMAND)
		{
			//DB_Log(6, "WM_COMMAND");
			HandleWmCommand((unsigned short)wParam);
		}

		return DefWindowProcA(hWnd, msg, wParam, (LPARAM)lParam);
	}

	if (msg == WM_CLOSE)
	{
		//DB_Log(6, "WM_CLOSE");

		receivedWmClose = true;
		PostQuitMessage(0);
		DoTheGame = false;

		return DefWindowProcA(hWnd, 0x10u, wParam, (LPARAM)lParam);
	}

	if (msg != WM_ACTIVATE)
	{
		return DefWindowProcA(hWnd, msg, wParam, (LPARAM)lParam);
	}

	//DB_Log(6, "WM_ACTIVATE");

	if (receivedWmClose)
	{
		return DefWindowProcA(hWnd, msg, wParam, (LPARAM)lParam);
	}

	//if (App_Unk00D9AC2B)
	//	return 0;

	if ((short)wParam)
	{
		if ((signed int)(unsigned short)wParam > 0 && (signed int)(unsigned short)wParam <= 2)
		{
			//DB_Log(6, "WM_ACTIVE");
			BlockAllInput = false;
			if (!Debug)
				ResumeThread((HANDLE)hThread);

			App_Unk00D9ABFD = 0;

			//DB_Log(5, "Game Thread Resumed");

			return 0;
		}
	}
	else
	{
		//DB_Log(6, "WM_INACTIVE");
		//DB_Log(5, "HangGameThread");
		BlockAllInput = true;
		App_Unk00D9ABFD = 1;

		if (!Debug)
			SuspendThread((HANDLE)hThread);

		//DB_Log(5, "Game Thread Suspended");
	}

	return 0;
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int RetVal;
	int n;

	// Process the command line
	bool setup = false;
	
	LPWSTR* argv;
	int argc;
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	for (int i = 1; i < argc; i++)
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
	
	// Initialise the new scripting system
	sol::state luaState;
	luaState.open_libraries(sol::lib::base);
	luaState.set_exception_handler(lua_exception_handler);

	g_GameFlow = new GameFlow(&luaState);
	LoadScript();

	g_GameScript = new GameScript(&luaState);

	// Initialise chunks for savegames
	SaveGame::Start();

	INITCOMMONCONTROLSEX commCtrlInit;
	commCtrlInit.dwSize = sizeof(INITCOMMONCONTROLSEX);
	commCtrlInit.dwICC = ICC_USEREX_CLASSES | ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&commCtrlInit);

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
		return false;
	}

	// Create the renderer and enumerate adapters and video modes
	g_Renderer = new Renderer11();
	g_Renderer->Create();
	g_Renderer->EnumerateVideoModes();

	// Load configuration and optionally show the setup dialog
	InitDefaultConfiguration();
	//SetupDialog();
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
		return false;
	}

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
	//Create debug script terminal
	if (Debug) {
		MainThreadID = GetWindowThreadProcessId(WindowsHandle, NULL);
		AllocConsole();
		HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
		DWORD consoleModeIn;
		int hCrt = _open_osfhandle((long)handle_in, _O_BINARY);
		FILE* hf_in = _fdopen(hCrt, "r");
		setvbuf(hf_in, NULL, _IONBF, 512);
		GetConsoleMode(handle_in, &consoleModeIn);
		consoleModeIn = consoleModeIn | ENABLE_LINE_INPUT;
		SetConsoleMode(handle_in, consoleModeIn);
		freopen_s(&hf_in, "CONIN$", "r", stdin);

		HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
		FILE* COutputHandle = _fdopen(SystemOutput, "w");
		freopen_s(&COutputHandle, "CONOUT$", "w", stdout);

		LPTHREAD_START_ROUTINE readConsoleLoop = [](LPVOID params) -> DWORD {
			DWORD read;
			CHAR buffer[4096];
			while (true) {
				BOOL success = ReadFile(params, &buffer, 4096, &read, NULL);
				if (success && read > 2) {
					//Only send the actual written message minus \r\n
					string msg(buffer, read-2);
					SendMessageA(WindowsHandle, WM_USER + 0, (WPARAM)&msg, NULL);
				}
			};
			return 0;
		};
		CreateThread(NULL, 0, readConsoleLoop, handle_in, 0, &DebugConsoleThreadID);
	}
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

int WinClose()
{
	//DB_Log(2, "WinClose - DLL");

	DestroyAcceleratorTable(hAccTable);

	if (g_Configuration.EnableSound)
		Sound_DeInit();
	
	delete g_Inventory;
	delete g_Renderer;
	delete g_GameScript;
	delete g_GameFlow;

	SaveGame::End();

	return 0;
}