#include "game.h"
#include "init.h"
#include "winmain.h"
#include <process.h>
#include <crtdbg.h>
#include <stdio.h>
#include "..\Game\draw.h"
#include "..\Game\sound.h"
#include "..\Game\inventory.h"
#include "..\Game\control.h"
#include "..\Game\gameflow.h"

WINAPP	 App;
unsigned int threadId;
uintptr_t hThread;
HACCEL hAccTable;

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

__int32 __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, __int32 nShowCmd)
{
	int RetVal;
	int n;

	// Clear Application Structure
	memset(&App, 0, sizeof(WINAPP));

	_CrtSetReportMode(0, 2);
	_CrtSetDbgFlag(-1);

	LoadGameflow();
	LoadSettings();

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

	tagRECT Rect;

	Rect.left = 0;
	Rect.top = 0;
	Rect.right = 800;
	Rect.bottom = 600;

	AdjustWindowRect(&Rect, WS_CAPTION, false);

	App.WindowHandle = CreateWindowEx(
		WS_THICKFRAME,
		"TR5Main",
		"TR5Main",
		WS_BORDER,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		Rect.right - Rect.left,
		Rect.bottom - Rect.top,
		nullptr,
		nullptr,
		App.hInstance,
		nullptr
	);

	if (!App.WindowHandle)
	{
		printf("Unable To Create Window");
		return FALSE;
	}

	// TODO: load settings from Windows registry
	OptionAutoTarget = 1;

	//DXInitialise(App.WindowHandle);
	g_Renderer = new Renderer();
	g_Renderer->Initialise(Rect.right, Rect.bottom, true, App.WindowHandle);

	// Initialize audio
	Sound_Init();

	// Initialise the new inventory
	g_Inventory = new Inventory();

	SetWindowPos(App.WindowHandle, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	WindowsHandle = App.WindowHandle; // GetDesktopWindow();

	App.bNoFocus = false;
	App.isInScene = false;

	UpdateWindow(WindowsHandle);
	ShowWindow(WindowsHandle, nShowCmd);

	SetCursor(0);
	ShowCursor(0);
	hAccTable = LoadAcceleratorsA(hInstance, (LPCSTR)0x65);

	SoundActive = false;
	DoTheGame = true;

	Unk_876C48 = false;
	hThread = _beginthreadex(0, 0, &GameMain, 0, 0, &threadId);
	WinProcMsg();
	Unk_876C48 = true;

	while (DoTheGame);
	
	//DXClose();
	//delete g_Renderer;

	WinClose();

	return 0;
}

__int32 __cdecl WinClose()
{
	DB_Log(2, "WinClose - DLL");
	// sub_401BEA(); Save register keys - future
	//CloseHandle(dxctx.cr.hObject);
	//sub_401AFF(&dxctx);
	DestroyAcceleratorTable(hAccTable);
	
//	Sound_DeInit();

	delete g_Renderer;
	delete g_Inventory;

	return 0;
}


void Inject_WinMain()
{
	INJECT(0x004D23E0, WinClose);
	INJECT(0x004D24C0, WinProcMsg);
	INJECT(0x004D1C00, WinMain);
}