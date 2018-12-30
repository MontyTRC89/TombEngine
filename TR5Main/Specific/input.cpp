#include "input.h"
#include "..\Global\global.h"
#include <stdio.h>

/*
int keyMapping[] = {
	-1,		VK_ESCAPE,	VK_,		"2",		"3",		"4",		"5",		"6",
	"7",		"8",		"9",		"0",		"-",		"+",		"BKSP",	"TAB",
	"Q",		"W",		"E",		"R",		"T",		"Y",		"U",		"I",
	"O",		"P",		"<",		">",		"RET",	"CTRL",	"A",		"S",
	"D",		"F",		"G",		"H",		"J",		"K",		"L",		";",
	"'",		"`",		"SHIFT",	"#",		"Z",		"X",		"C",		"V",
	"B",		"N",		"M",		",",		".",		"/",		"SHIFT",	"PADx",
	"ALT",	"SPACE",	"CAPS",	-1,		-1,		-1,		-1,		-1,

	-1,		-1,		-1,		-1,		-1,		"NMLK",	-1,		"PAD7",
	"PAD8",	"PAD9",	"PAD-",	"PAD4",	"PAD5",	"PAD6",	"PAD+",	"PAD1",
	"PAD2",	"PAD3",	"PAD0",	"PAD.",	-1,		-1,		"\\",		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,

	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		"ENTER",	"CTRL",	-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		"SHIFT",	-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		"PAD/",	-1,		-1,
	"ALT",	-1,		-1,		-1,		-1,		-1,		-1,		-1,

	-1,		-1,		-1,		-1,		-1,		-1,		-1,		"HOME",
	"UP",		"PGUP",	-1,		"LEFT",	-1,		"RIGHT",	-1,		"END",
	"DOWN",	"PGDN",	"INS",	"DEL",	-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,
	-1,		-1,		-1,		-1,		-1,		-1,		-1,		-1,

	"JOY1", 	"JOY2",		"JOY3",		"JOY4", 	"JOY5",		"JOY6", 	"JOY7",		"JOY8",
	"JOY9",		"JOY10",	"JOY11",	"JOY12",	"JOY13",	"JOY14",	"JOY15",	"JOY16"
};*/

void __cdecl InitialiseDirectInput(HWND handle, HINSTANCE instance)
{
	// Dummy function, we don't need DirectInput anymore
}

void __cdecl DI_ReadKeyboard(byte* keys)
{
	for (int i = 0; i < 256; i++)
		keys[i] = GetAsyncKeyState(MapVirtualKey(i, MAPVK_VSC_TO_VK)) >> 8;

	keys[TR_KEY_LEFT] = GetAsyncKeyState(VK_LEFT) >> 8;
	keys[TR_KEY_RIGHT] = GetAsyncKeyState(VK_RIGHT) >> 8;
	keys[TR_KEY_UP] = GetAsyncKeyState(VK_UP) >> 8;
	keys[TR_KEY_DOWN] = GetAsyncKeyState(VK_DOWN) >> 8;
	keys[TR_KEY_PRIOR] = GetAsyncKeyState(VK_PRIOR) >> 8;
	keys[TR_KEY_NEXT] = GetAsyncKeyState(VK_NEXT) >> 8;
	keys[TR_KEY_END] = GetAsyncKeyState(VK_END) >> 8;
	keys[TR_KEY_NUMPAD0] = GetAsyncKeyState(VK_NUMPAD0) >> 8;
}

__int32 __cdecl DD_SpinMessageLoopMaybe()
{
	return 0;
}

void Inject_Input()
{
	INJECT(0x004A2970, InitialiseDirectInput);
	INJECT(0x004A2880, DI_ReadKeyboard);
	INJECT(0x004A2D00, DD_SpinMessageLoopMaybe);
}