#include "framework.h"
#include "Specific/winmain.h"

#include <CommCtrl.h>
#include <process.h>
#include <resource.h>
#include <iostream>
#include <codecvt>
#include <filesystem>

#include "Game/control/control.h"
#include "Game/savegame.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/configuration.h"
#include "Specific/trutils.h"
#include "Scripting/Internal/LanguageScript.h"
#include "Scripting/Include/ScriptInterfaceState.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"

using namespace TEN::Renderer;
using namespace TEN::Input;
using namespace TEN::Utils;

using std::exception;
using std::string;
using std::cout;
using std::endl;

WINAPP App;
unsigned int ThreadID;
uintptr_t ThreadHandle;
HACCEL hAccTable;
bool DebugMode = false;
HWND WindowsHandle;
DWORD MainThreadID;

// Indicates to hybrid graphics systems to prefer discrete part by default.
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

bool ArgEquals(wchar_t* incomingArg, std::string name)
{
	auto lowerArg = TEN::Utils::ToLower(TEN::Utils::ToString(incomingArg));
	return (lowerArg == "-" + name) || (lowerArg == "/" + name);
}

Vector2i GetScreenResolution()
{
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	Vector2i resolution;
	resolution.x = desktop.right;
	resolution.y = desktop.bottom;
	return resolution;
}

std::vector<Vector2i> GetAllSupportedScreenResolutions()
{
	auto resList = std::vector<Vector2i>{};

	DEVMODE dm = { 0 };
	dm.dmSize = sizeof(dm);
	for (int iModeNum = 0; EnumDisplaySettings(NULL, iModeNum, &dm) != 0; iModeNum++)
	{
		bool add = true;
		for (auto m : resList)
		{
			if (m.x == dm.dmPelsWidth && m.y == dm.dmPelsHeight)
			{
				add = false;
				break;
			}
		}
		if (add)
		{
			auto res = Vector2i(dm.dmPelsWidth, dm.dmPelsHeight);
			resList.push_back(res);
		}
	}

	std::sort(
		resList.begin(), resList.end(),
		[](Vector2i& a, Vector2i& b)
		{
			return ((a.x == b.x) ? (a.y < b.y) : (a.x < b.x));
		});

	return resList;
}

void DisableDpiAwareness()
{
	// Don't use SHCore library directly, as it's not available on pre-win 8.1 systems.

	typedef HRESULT(WINAPI* SetDpiAwarenessProc)(UINT);
	static constexpr unsigned int PROCESS_SYSTEM_DPI_AWARE = 1;

	auto lib = LoadLibrary("SHCore.dll");
	if (lib == NULL)
		return;

	auto setDpiAwareness = (SetDpiAwarenessProc)GetProcAddress(lib, "SetProcessDpiAwareness");
	if (setDpiAwareness == NULL)
		return;

	setDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	FreeLibrary(lib);
}

void WinProcMsg()
{
	MSG Msg;

	do
	{
		GetMessage(&Msg, 0, 0, 0);
		if (!TranslateAccelerator(WindowsHandle, hAccTable, &Msg))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}
	while (!ThreadEnded && Msg.message != WM_QUIT);
}

void CALLBACK HandleWmCommand(unsigned short wParam)
{
	if (wParam == WM_KILLFOCUS)
	{
		// make sure we suspend the game (if focus is removed) only if the level is not being loaded
		
		if (!LevelLoadTask.valid())
		{
			SuspendThread((HANDLE)ThreadHandle);
			g_Renderer.ToggleFullScreen();
			ResumeThread((HANDLE)ThreadHandle);

			if (g_Renderer.IsFullsScreen())
			{
				SetCursor(nullptr);
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
	static bool receivedWmClose = false;

	// Disables ALT + SPACE
	if (msg == WM_SYSCOMMAND && wParam == SC_KEYMENU)
	{
		return 0;
	}

	if (msg > WM_CLOSE)
	{
		if (msg == WM_COMMAND)
			HandleWmCommand((unsigned short)wParam);

		return DefWindowProcA(hWnd, msg, wParam, (LPARAM)lParam);
	}

	if (msg == WM_CLOSE)
	{
		receivedWmClose = true;
		PostQuitMessage(0);
		DoTheGame = false;

		return DefWindowProcA(hWnd, 0x10u, wParam, (LPARAM)lParam);
	}

	if (msg != WM_ACTIVATE)
		return DefWindowProcA(hWnd, msg, wParam, (LPARAM)lParam);

	if (receivedWmClose)
	{
		return DefWindowProcA(hWnd, msg, wParam, (LPARAM)lParam);
	}

	if ((short)wParam)
	{
		if ((signed int)(unsigned short)wParam > 0 && (signed int)(unsigned short)wParam <= 2)
		{
			if (!g_Configuration.EnableWindowedMode)
				g_Renderer.ToggleFullScreen(true);

			if (!DebugMode && ThreadHandle > 0)
			{
				TENLog("Resuming game thread", LogLevel::Info);
				ResumeThread((HANDLE)ThreadHandle);
				ResumeAllSounds(SoundPauseMode::Global);
			}

			return 0;
		}
	}
	else
	{
		if (!g_Configuration.EnableWindowedMode)
			ShowWindow(hWnd, SW_MINIMIZE);

		if (!DebugMode)
		{
			TENLog("Suspending game thread", LogLevel::Info);
			SuspendThread((HANDLE)ThreadHandle);
			PauseAllSounds(SoundPauseMode::Global);
		}
	}

	return 0;
}

int main() 
{
	return WinMain(GetModuleHandle(nullptr), nullptr, GetCommandLine(), SW_SHOW);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// Process command line arguments.
	bool setup = false;
	std::string levelFile = {};
	LPWSTR* argv;
	int argc;
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	std::string gameDir{};

	// Parse command line arguments.
	for (int i = 1; i < argc; i++)
	{
		if (ArgEquals(argv[i], "setup"))
		{
			setup = true;
		}
		else if (ArgEquals(argv[i], "debug"))
		{
			DebugMode = true;
		}
		else if (ArgEquals(argv[i], "level") && argc > (i + 1))
		{
			levelFile = TEN::Utils::ToString(argv[i + 1]);
		}
		else if (ArgEquals(argv[i], "hash") && argc > (i + 1))
		{
			SystemNameHash = std::stoul(std::wstring(argv[i + 1]));
		}
		else if (ArgEquals(argv[i], "gamedir") && argc > (i + 1))
		{
			gameDir = TEN::Utils::ToString(argv[i + 1]);
		}
	}
	LocalFree(argv);

	// Construct asset directory.
	gameDir = ConstructAssetDirectory(gameDir);

	// Hide console window if mode isn't debug.
#ifndef _DEBUG
	if (!DebugMode)
		ShowWindow(GetConsoleWindow(), 0);
#endif

	// Clear application structure.
	memset(&App, 0, sizeof(WINAPP));
	
	// Initialize logging.
	InitTENLog(gameDir);

	// Indicate version.
	auto ver = GetProductOrFileVersion(false);
	auto windowName = (std::string("Starting TombEngine version ") +
					   std::to_string(ver[0]) + "." +
					   std::to_string(ver[1]) + "." +
					   std::to_string(ver[2]) + " " +
#ifdef _WIN64
					   "(64-bit)"
#else
					   "(32-bit)"
#endif
					   );
	TENLog(windowName, LogLevel::Info);

	// Initialize savegame and scripting systems.
	SaveGame::Init(gameDir);
	ScriptInterfaceState::Init(gameDir);

	// Initialize scripting.
	try 
	{
		g_GameFlow = ScriptInterfaceState::CreateFlow();
		g_GameScriptEntities = ScriptInterfaceState::CreateObjectsHandler();
		g_GameStringsHandler = ScriptInterfaceState::CreateStringsHandler();

		// This must be loaded last as it adds metafunctions to the global
		// table so that every global variable added henceforth gets put
		// into a special hidden table which we can clean up.
		// By doing this last, we ensure that all built-in usertypes
		// are added to a hierarchy in the REAL global table, not the fake
		// hidden one.
		g_GameScript = ScriptInterfaceState::CreateGame();

		//todo Major hack. This should not be needed to leak outside of
		//LogicHandler internals. In a future version stuff from FlowHandler
		//should be moved to LogicHandler or vice versa to make this stuff
		//less fragile (squidshire, 16/09/22)
		g_GameScript->ShortenTENCalls();
		g_GameFlow->SetGameDir(gameDir);
		g_GameFlow->LoadFlowScript();
	}
	catch (TENScriptException const& e)
	{
		std::string msg = std::string{ "A Lua error occurred while setting up scripts; " } + __func__ + ": " + e.what();
		TENLog(msg, LogLevel::Error, LogConfig::All);
		ShutdownTENLog();
		return 0;
	}

	// Disable DPI scaling on Windows 8.1+ systems.
	DisableDpiAwareness();

	// Set up main window.
	INITCOMMONCONTROLSEX commCtrlInit;
	commCtrlInit.dwSize = sizeof(INITCOMMONCONTROLSEX);
	commCtrlInit.dwICC = ICC_USEREX_CLASSES | ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&commCtrlInit);

	// Initialize main window.
	App.hInstance = hInstance;
	App.WindowClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	App.WindowClass.lpszMenuName = NULL;
	App.WindowClass.lpszClassName = "TombEngine";
	App.WindowClass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	App.WindowClass.hInstance = hInstance;
	App.WindowClass.style = CS_VREDRAW | CS_HREDRAW;
	App.WindowClass.lpfnWndProc = WinAppProc;
	App.WindowClass.cbClsExtra = 0;
	App.WindowClass.cbWndExtra = 0;
	App.WindowClass.hCursor = LoadCursor(App.hInstance, IDC_ARROW);

	// Register main window.
	if (!RegisterClass(&App.WindowClass))
	{
		TENLog("Unable to register window class.", LogLevel::Error);
		return 0;
	}

	// Create renderer and enumerate adapters and video modes.
	g_Renderer.Create();

	// Load configuration and optionally show setup dialog.
	InitDefaultConfiguration();
	if (setup || !LoadConfiguration())
	{
		if (!SetupDialog())
		{
			WinClose();
			return 0;
		}

		LoadConfiguration();
	}

	// Set up window dimensions.
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = g_Configuration.ScreenWidth;
	rect.bottom = g_Configuration.ScreenHeight;
	AdjustWindowRect(&rect, WS_CAPTION, false);

	// Calculate window resolution.
	auto windowRes = Vector2i(rect.right - rect.left, rect.bottom - rect.top);

	// Get screen resolution of primary monitor.
	auto screenRes = Vector2i(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	// Calculate centered window position on screen.
	auto windowPos = (screenRes - windowRes) / 2;

	// Create window handle.
	App.WindowHandle = CreateWindowEx(
		0,
		"TombEngine",
		g_GameFlow->GetString(STRING_WINDOW_TITLE),
		WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX,
		windowPos.x,
		windowPos.y,
		windowRes.x,
		windowRes.y,
		NULL,
		NULL,
		App.hInstance,
		NULL);

	// Register window handle.
	if (!App.WindowHandle)
	{
		TENLog("Unable to create Window. Error: " + std::to_string(GetLastError()), LogLevel::Error);
		return 0;
	}
	else
	{
		WindowsHandle = App.WindowHandle;
	}

	try
	{
		// Unlike CoInitialize(), this line prevents event spamming if a .dll fails.
		CoInitializeEx(NULL, COINIT_MULTITHREADED);

		// Initialize renderer.
		g_Renderer.Initialize(g_Configuration.ScreenWidth, g_Configuration.ScreenHeight, g_Configuration.EnableWindowedMode, App.WindowHandle);

		// Initialize audio.
		Sound_Init(gameDir);

		// Initialize input.
		InitializeInput(App.WindowHandle);

		// Load level if specified in command line.
		CurrentLevel = g_GameFlow->GetLevelNumber(levelFile);

		App.bNoFocus = false;
		App.isInScene = false;

		UpdateWindow(WindowsHandle);
		ShowWindow(WindowsHandle, nShowCmd);

		SetCursor(NULL);
		ShowCursor(FALSE);
		hAccTable = LoadAccelerators(hInstance, (LPCSTR)0x65);
	}
	catch (std::exception& ex)
	{
		TENLog("Error during game initialization: " + std::string(ex.what()), LogLevel::Error);
		WinClose();
		exit(EXIT_FAILURE);
	}

	DoTheGame = true;

	ThreadEnded = false;
	ThreadHandle = BeginThread(GameMain, ThreadID);

	// The game window likes to steal input anyway, so let's put it at the
	// foreground so the user at least expects it.
	if (GetForegroundWindow() != WindowsHandle)
		SetForegroundWindow(WindowsHandle);

	WinProcMsg();
	ThreadEnded = true;

	while (DoTheGame);

	WinClose();
	exit(EXIT_SUCCESS);
}

void WinClose()
{
	WaitForSingleObject((HANDLE)ThreadHandle, 5000);

	DestroyAcceleratorTable(hAccTable);

	Sound_DeInit();
	DeinitializeInput();

	TENLog("Cleaning up and exiting...", LogLevel::Info);
	
	delete g_GameScript;
	g_GameScript = nullptr;

	delete g_GameFlow;
	g_GameFlow = nullptr;

	delete g_GameScriptEntities;
	g_GameScriptEntities = nullptr;

	delete g_GameStringsHandler;
	g_GameStringsHandler = nullptr;

	ShutdownTENLog();

	CoUninitialize();
}
