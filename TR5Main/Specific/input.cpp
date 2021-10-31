#include "framework.h"
#include "input.h"
#include "Lara.h"
#include "winmain.h"
#include "camera.h"
#include "Sound\sound.h"
#include "savegame.h"
#include "Renderer11.h"
#include "items.h"

using TEN::Renderer::g_Renderer;

const char* g_KeyNames[] = {
	NULL,		"ESC",	"1",		"2",		"3",		"4",		"5",		"6",
		"7",		"8",		"9",		"0",		"-",		"+",		"BKSP",	"TAB",
		"Q",		"W",		"E",		"R",		"T",		"Y",		"U",		"I",
		"O",		"P",		"<",		">",		"RET",	"CTRL",	"A",		"S",
		"D",		"F",		"G",		"H",		"J",		"K",		"L",		";",
		"'",		"`",		"SHIFT",	"#",		"Z",		"X",		"C",		"V",
		"B",		"N",		"M",		",",		".",		"/",		"SHIFT",	"PADx",
		"ALT",	"SPACE",	"CAPS",	NULL,		NULL,		NULL,		NULL,		NULL,

		NULL,		NULL,		NULL,		NULL,		NULL,		"NMLK",	NULL,		"PAD7",
		"PAD8",	"PAD9",	"PAD-",	"PAD4",	"PAD5",	"PAD6",	"PAD+",	"PAD1",
		"PAD2",	"PAD3",	"PAD0",	"PAD.",	NULL,		NULL,		"\\",		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,

		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		"ENTER",	"CTRL",	NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		"SHIFT",	NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		"PAD/",	NULL,		NULL,
		"ALT",	NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,

		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		"HOME",
		"UP",		"PGUP",	NULL,		"LEFT",	NULL,		"RIGHT",	NULL,		"END",
		"DOWN",	"PGDN",	"INS",	"DEL",	NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,

		"JOY1", 	"JOY2",		"JOY3",		"JOY4", 	"JOY5",		"JOY6", 	"JOY7",		"JOY8",
		"JOY9",		"JOY10",	"JOY11",	"JOY12",	"JOY13",	"JOY14",	"JOY15",	"JOY16"
};

int TrInput;
int DbInput;
int InputBusy;
bool SetDebounce = false;

short MouseX;
short MouseY;
int MouseKeys;

byte KeyMap[256];
int ConflictingKeys[18];
short KeyboardLayout[2][18] =
{
	{ DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT, DIK_PERIOD, DIK_SLASH, DIK_RSHIFT, DIK_RMENU, DIK_RCONTROL, DIK_SPACE, DIK_COMMA, DIK_NUMPAD0, DIK_END, DIK_ESCAPE, DIK_DELETE, DIK_NEXT, DIK_P, DIK_RETURN },
	{ DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT, DIK_PERIOD, DIK_SLASH, DIK_RSHIFT, DIK_RMENU, DIK_RCONTROL, DIK_SPACE, DIK_COMMA, DIK_NUMPAD0, DIK_END, DIK_ESCAPE, DIK_DELETE, DIK_NEXT, DIK_P, DIK_RETURN }
};

int joy_x;
int joy_y;
int joy_fire;

void InitialiseDirectInput(HWND handle, HINSTANCE instance)
{
	// Dummy function, we don't need DirectInput anymore
}

void DI_ReadKeyboard(byte* keys)
{
	for (int i = 0; i < 256; i++)
		keys[i] = GetAsyncKeyState(MapVirtualKey(i, MAPVK_VSC_TO_VK)) >> 8;

	// Empty the numeric pad keys
	keys[DIK_DECIMAL] = 0;
	keys[DIK_NUMPAD0] = 0;
	keys[DIK_NUMPAD1] = 0;
	keys[DIK_NUMPAD2] = 0;
	keys[DIK_NUMPAD3] = 0;
	keys[DIK_NUMPAD4] = 0;
	keys[DIK_NUMPAD5] = 0;
	keys[DIK_NUMPAD6] = 0;
	keys[DIK_NUMPAD7] = 0;
	keys[DIK_NUMPAD8] = 0;
	keys[DIK_NUMPAD9] = 0;
	
	// Some keys are not mapped by MapVirtualKey()
	keys[DIK_LEFT] = GetAsyncKeyState(VK_LEFT) >> 8;
	keys[DIK_RIGHT] = GetAsyncKeyState(VK_RIGHT) >> 8;
	keys[DIK_UP] = GetAsyncKeyState(VK_UP) >> 8;
	keys[DIK_DOWN] = GetAsyncKeyState(VK_DOWN) >> 8;
	keys[DIK_PRIOR] = GetAsyncKeyState(VK_PRIOR) >> 8;
	keys[DIK_NEXT] = GetAsyncKeyState(VK_NEXT) >> 8;
	keys[DIK_END] = GetAsyncKeyState(VK_END) >> 8;
	keys[DIK_HOME] = GetAsyncKeyState(VK_HOME) >> 8;
	keys[DIK_INSERT] = GetAsyncKeyState(VK_INSERT) >> 8;
	keys[DIK_DELETE] = GetAsyncKeyState(VK_DELETE) >> 8;
	keys[DIK_NUMPAD0] = GetAsyncKeyState(VK_NUMPAD0) >> 8;
}

int DD_SpinMessageLoopMaybe()
{
	return 0;
}

int Key(int number)
{
	short key = KeyboardLayout[1][number];

	if (number >= 256)
		return joy_fire & (1 << number);

	if (KeyMap[key])
		return 1;

	switch (key)
	{
	case DIK_RCONTROL:
		return KeyMap[DIK_LCONTROL];
	case DIK_LCONTROL:
		return KeyMap[DIK_RCONTROL];
	case DIK_RSHIFT:
		return KeyMap[DIK_LSHIFT];
	case DIK_LSHIFT:
		return KeyMap[DIK_RSHIFT];
	case DIK_RMENU:
		return KeyMap[DIK_LMENU];
	case DIK_LMENU:
		return KeyMap[DIK_RMENU];
	}

	if (ConflictingKeys[number])
		return 0;

	key = KeyboardLayout[0][number];

	if (KeyMap[key])
		return 1;

	switch (key)
	{
	case DIK_RCONTROL:
		return KeyMap[DIK_LCONTROL];
	case DIK_LCONTROL:
		return KeyMap[DIK_RCONTROL];
	case DIK_RSHIFT:
		return KeyMap[DIK_LSHIFT];
	case DIK_LSHIFT:
		return KeyMap[DIK_RSHIFT];
	case DIK_RMENU:
		return KeyMap[DIK_LMENU];
	case DIK_LMENU:
		return KeyMap[DIK_RMENU];
	}

	return 0;
}

int S_UpdateInput()
{
	DI_ReadKeyboard(KeyMap);

	static int linput;

	linput = 0;

	if (Key(KEY_FORWARD))
		linput |= IN_FORWARD;
	if (Key(KEY_BACK))
		linput |= IN_BACK;
	if (Key(KEY_LEFT))
		linput |= IN_LEFT;
	if (Key(KEY_RIGHT))
		linput |= IN_RIGHT;
	if (Key(KEY_DUCK))
		linput |= IN_DUCK;
	if (Key(KEY_SPRINT))
		linput |= IN_SPRINT;
	if (Key(KEY_WALK))
		linput |= IN_WALK;
	if (Key(KEY_JUMP))
		linput |= IN_JUMP;
	if (Key(KEY_ACTION))
		linput |= IN_ACTION | IN_SELECT;
	if (Key(KEY_DRAW))
		linput |= IN_DRAW;

	bool flare = false;
	static bool flare_no = false;

	/*if (opt_ControlMethod == CM_JOYSTICK)
	{
		if (Key(opt_JJmp + 256))
			linput |= IN_JUMP;
		if (Key(opt_JAct + 256))
			linput |= IN_ACTION | IN_SELECT;
		if (Key(opt_JDrw + 256))
			linput |= IN_DRAW;
		if (Key(opt_JDsh + 256))
			linput |= IN_SPRINT;
		if (Key(opt_JWlk + 256))
			linput |= IN_WALK;
		if (Key(opt_JDck + 256))
			linput |= IN_DUCK;
		if (Key(opt_JFlr + 256))
			flare = true;
	}*/

	if (Key(KEY_FLARE) || flare)
	{
		if (!flare_no)
		{
			if (LaraItem->currentAnimState == LS_CRAWL_IDLE ||
				LaraItem->currentAnimState == LS_CRAWL_FORWARD ||
				LaraItem->currentAnimState == LS_CRAWL_TURN_LEFT ||
				LaraItem->currentAnimState == LS_CRAWL_TURN_RIGHT ||
				LaraItem->currentAnimState == LS_CRAWL_BACK ||
				LaraItem->currentAnimState == LS_CRAWL_TO_HANG)
			{
				SoundEffect(SFX_TR4_LARA_NO, nullptr, 2);
				flare_no = true;
			}
			else
			{
				flare_no = false;
				linput |= IN_FLARE;
			}
		}
	}
	else
	{
		flare_no = false;
	}

	if (Key(KEY_LOOK))
		linput |= IN_LOOK;
	if (Key(KEY_ROLL))
		linput |= IN_ROLL;
	if (Key(KEY_OPTION))
		linput |= IN_OPTION;
	if (Key(KEY_STEPL))
		linput |= IN_WALK | IN_LEFT;
	if (Key(KEY_STEPR))
		linput |= IN_WALK | IN_RIGHT;
	if (Key(KEY_PAUSE))
		linput |= IN_PAUSE;
	if (Key(KEY_SELECT))
		linput |= IN_SELECT;

	/*if (opt_ControlMethod == CM_JOYSTICK)
	{
		if (Key(opt_JLok + 256))
			linput |= IN_LOOK;
		if (Key(opt_JRol + 256))
			linput |= IN_ROLL;
		if (Key(opt_JInv + 256))
			linput |= IN_OPTION;
	}*/

	// CHECK
	if (KeyMap[1])
		linput |= IN_OPTION | IN_DESELECT;
	
	// Switch debug pages

	static int debug_timeout = 0;
	if (KeyMap[DIK_F10] || KeyMap[DIK_F11])
	{
		if (debug_timeout == 0)
		{
			debug_timeout = 1;
			g_Renderer.switchDebugPage(KeyMap[DIK_F10]);
		}
	}
	else
		debug_timeout = 0;

	static int look_timeout = 0;

	if (Lara.gunStatus == LG_READY)
	{
		// TODO: AutoTarget Not Saved
		//Savegame.AutoTarget = 1;

		if (linput & IN_LOOK)
		{
			if (look_timeout >= 6)
			{
				look_timeout = 100;
			}
			else
			{
				linput &= ~IN_LOOK;
				look_timeout++;
			}
		}
		else
		{
			if (look_timeout != 0 && look_timeout != 100)
			{
				linput |= IN_LOOKSWITCH;
			}

			look_timeout = 0;
		}
	}

	static int medipack_timeout = 0;

	/***************************WEAPON HOTKEYS***************************/
	if (KeyMap[DIK_1] && Lara.Weapons[WEAPON_PISTOLS].Present == true)
		Lara.requestGunType = WEAPON_PISTOLS;
	if (KeyMap[DIK_2] && Lara.Weapons[WEAPON_SHOTGUN].Present == true)
		Lara.requestGunType = WEAPON_SHOTGUN;
	if (KeyMap[DIK_3] && Lara.Weapons[WEAPON_REVOLVER].Present == true)
		Lara.requestGunType = WEAPON_REVOLVER;
	if (KeyMap[DIK_4] && Lara.Weapons[WEAPON_UZI].Present == true)
		Lara.requestGunType = WEAPON_UZI;
	if (KeyMap[DIK_5] && Lara.Weapons[WEAPON_HARPOON_GUN].Present == true)
		Lara.requestGunType = WEAPON_HARPOON_GUN;
	if (KeyMap[DIK_6] && Lara.Weapons[WEAPON_HK].Present == true)
		Lara.requestGunType = WEAPON_HK;
	if (KeyMap[DIK_7] && Lara.Weapons[WEAPON_ROCKET_LAUNCHER].Present == true)
		Lara.requestGunType = WEAPON_ROCKET_LAUNCHER;
	if (KeyMap[DIK_8] && Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Present == true)
		Lara.requestGunType = WEAPON_GRENADE_LAUNCHER;
		/*------------------------------------------------------------------*/

	if (KeyMap[DIK_0])
	{
		if (medipack_timeout == 0)
		{
			if (LaraItem->hitPoints > 0 && LaraItem->hitPoints < 1000 || Lara.poisoned)
			{
				if (Lara.NumSmallMedipacks != 0)
				{
					if (Lara.NumSmallMedipacks != -1)
						Lara.NumSmallMedipacks--;

					Lara.poisoned = 0;
					LaraItem->hitPoints += 500;
					SoundEffect(SFX_TR4_MENU_MEDI, nullptr, 2);//Fix heal sound not triggering if small medi doesn't top off Lara's health. original tr4/5 issue

					if (LaraItem->hitPoints > 1000)
					{
						LaraItem->hitPoints = 1000;
						SoundEffect(SFX_TR4_MENU_MEDI, nullptr, 2);
						Statistics.Game.HealthUsed++;
					}
				}

				medipack_timeout = 15;
			}
		}
	}
	else if (KeyMap[DIK_9])
	{
		if (medipack_timeout == 0)
		{
			if (LaraItem->hitPoints > 0 && LaraItem->hitPoints < 1000 || Lara.poisoned)
			{
				if (Lara.NumLargeMedipacks != 0)
				{
					if (Lara.NumLargeMedipacks != -1)
						Lara.NumLargeMedipacks--;

					Lara.poisoned = 0;
					LaraItem->hitPoints += 1000;

					if (LaraItem->hitPoints > 1000)
					{
						LaraItem->hitPoints = 1000;
						SoundEffect(SFX_TR4_MENU_MEDI, nullptr, 2);
						Statistics.Game.HealthUsed++;
					}
				}

				medipack_timeout = 15;
			}
		}
	}
	else if (medipack_timeout != 0)
	{
		medipack_timeout--;
	}

	if (KeyMap[DIK_F10])
		linput |= IN_E;

	if (linput & IN_WALK && !(linput & IN_FORWARD) && !(linput & IN_BACK))
	{
		if (linput & IN_LEFT)
		{
			linput &= ~IN_LEFT;
			linput |= IN_LSTEP;
		}
		else if (linput & IN_RIGHT)
		{
			linput &= ~IN_RIGHT;
			linput |= IN_RSTEP;
		}
	}

	if (linput & IN_FORWARD && linput & IN_BACK)
		linput |= IN_ROLL;

	if (linput & IN_ROLL && BinocularRange)
		linput &= ~IN_ROLL;

	if (linput & IN_LEFT && linput & IN_RIGHT)
		linput &= ~(IN_LEFT | IN_RIGHT);

	if (SetDebounce)
		DbInput = InputBusy;

	if (KeyMap[DIK_F5])
		linput |= IN_SAVE;

	if (KeyMap[DIK_F6])
		linput |= IN_LOAD;

	/*if (Gameflow->CheatEnabled)
	{
		static int cheat_code = 0;

		if (linput != 0)
			cheat_code = 0;

		switch (cheat_code)
		{
		case 0:
			if (Key(DIK_D))
				cheat_code = 1;
			break;
		case 1:
			if (Key(DIK_O))
				cheat_code = 2;
			break;
		case 2:
			if (Key(DIK_Z))
				cheat_code = 3;
			break;
		case 3:
			if (Key(DIK_Y))
				linput = IN_CHEAT;
			break;
		}
	}*/

	InputBusy = linput;
	TrInput = linput;

	if (Lara.busy)
	{
		linput &= (IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_OPTION | IN_LOOK | IN_PAUSE);

		if (linput & IN_FORWARD && linput & IN_BACK)
			linput &= ~IN_BACK;
	}

	if (SetDebounce)
		DbInput = TrInput & (DbInput ^ TrInput);

	if (KeyMap[DIK_F])
		linput |= IN_FORWARD;
	else
		linput &= ~IN_FORWARD;

	//dbginput = TrInput;

	return true;
}

void DefaultConflict()
{
	for (int i = 0; i < NUM_CONTROLS; i++)
	{
		short key = KeyboardLayout[0][i];

		ConflictingKeys[i] = false;

		for (int j = 0; j < NUM_CONTROLS; j++)
		{
			if (key == KeyboardLayout[1][j])
			{
				ConflictingKeys[i] = true;
				break;
			}
		}
	}
}
