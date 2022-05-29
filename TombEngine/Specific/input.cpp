#include "framework.h"
#include "Specific/input.h"

#include "Game/camera.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/savegame.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/winmain.h"

using TEN::Renderer::g_Renderer;

const char* g_KeyNames[] =
{
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

int JoyX;
int JoyY;
int JoyFire;

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

bool Key(int number)
{
	short key = KeyboardLayout[1][number];

	if (number >= 256)
		return JoyFire & (1 << number);

	if (KeyMap[key])
		return true;

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
		return false;

	key = KeyboardLayout[0][number];

	if (KeyMap[key])
		return true;

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

	return false;
}

int S_UpdateInput()
{
	DI_ReadKeyboard(KeyMap);

	static int lInput;

	lInput = 0;

	if (Key(KEY_FORWARD))
		lInput |= IN_FORWARD;

	if (Key(KEY_BACK))
		lInput |= IN_BACK;

	if (Key(KEY_LEFT))
		lInput |= IN_LEFT;

	if (Key(KEY_RIGHT))
		lInput |= IN_RIGHT;

	if (Key(KEY_CROUCH))
		lInput |= IN_CROUCH;

	if (Key(KEY_SPRINT))
		lInput |= IN_SPRINT;

	if (Key(KEY_WALK))
		lInput |= IN_WALK;

	if (Key(KEY_JUMP))
		lInput |= IN_JUMP;

	if (Key(KEY_ACTION))
		lInput |= IN_ACTION | IN_SELECT;

	if (Key(KEY_DRAW))
		lInput |= IN_DRAW;

	bool flare = false;
	static bool flareNo = false;

	/*if (opt_ControlMethod == CM_JOYSTICK)
	{
		if (Key(opt_JJmp + 256))
			lInput |= IN_JUMP;

		if (Key(opt_JAct + 256))
			lInput |= IN_ACTION | IN_SELECT;

		if (Key(opt_JDrw + 256))
			lInput |= IN_DRAW;

		if (Key(opt_JDsh + 256))
			lInput |= IN_SPRINT;

		if (Key(opt_JWlk + 256))
			lInput |= IN_WALK;

		if (Key(opt_JDck + 256))
			lInput |= IN_CROUCH;

		if (Key(opt_JFlr + 256))
			flare = true;
	}*/

	// TODO: Better flare handling in crawl states.
	if (Key(KEY_FLARE) || flare)
	{
		if (!flareNo)
		{
			if (LaraItem->Animation.ActiveState == LS_CRAWL_FORWARD ||
				LaraItem->Animation.ActiveState == LS_CRAWL_TURN_LEFT ||
				LaraItem->Animation.ActiveState == LS_CRAWL_TURN_RIGHT ||
				LaraItem->Animation.ActiveState == LS_CRAWL_BACK ||
				LaraItem->Animation.ActiveState == LS_CRAWL_TO_HANG)
			{
				SoundEffect(SFX_TR4_LARA_NO, nullptr, SoundEnvironment::Always);
				flareNo = true;
			}
			else
			{
				flareNo = false;
				lInput |= IN_FLARE;
			}
		}
	}
	else
		flareNo = false;

	if (Key(KEY_LOOK))
		lInput |= IN_LOOK;

	if (Key(KEY_ROLL))
		lInput |= IN_ROLL;

	if (Key(KEY_OPTION))
		lInput |= IN_OPTION;

	if (Key(KEY_LSTEP))
		lInput |= IN_LSTEP;

	if (Key(KEY_RSTEP))
		lInput |= IN_RSTEP;

	if (Key(KEY_PAUSE))
		lInput |= IN_PAUSE;

	if (Key(KEY_SELECT))
		lInput |= IN_SELECT;

	/*if (opt_ControlMethod == CM_JOYSTICK)
	{
		if (Key(opt_JLok + 256))
			lInput |= IN_LOOK;

		if (Key(opt_JRol + 256))
			lInput |= IN_ROLL;

		if (Key(opt_JInv + 256))
			lInput |= IN_OPTION;
	}*/

	// CHECK
	if (KeyMap[1])
		lInput |= IN_OPTION | IN_DESELECT;
	
	// Switch debug pages

	static int debugTimeout = 0;
	if (KeyMap[DIK_F10] || KeyMap[DIK_F11])
	{
		if (debugTimeout == 0)
		{
			debugTimeout = 1;
			g_Renderer.SwitchDebugPage(KeyMap[DIK_F10]);
		}
	}
	else
		debugTimeout = 0;

	static int lookTimeout = 0;

	if (Lara.Control.HandStatus == HandStatus::WeaponReady)
	{
		// TODO: AutoTarget Not Saved
		//Savegame.AutoTarget = 1;

		if (lInput & IN_LOOK)
		{
			if (lookTimeout >= 6)
				lookTimeout = 100;
			else
			{
				lInput &= ~IN_LOOK;
				lookTimeout++;
			}
		}
		else
		{
			if (lookTimeout != 0 && lookTimeout != 100)
				lInput |= IN_LOOKSWITCH;

			lookTimeout = 0;
		}
	}

	static int medipackTimeout = 0;

	/***************************WEAPON HOTKEYS***************************/
	if (KeyMap[DIK_1] && Lara.Weapons[(int)LaraWeaponType::Pistol].Present == true)
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::Pistol;

	if (KeyMap[DIK_2] && Lara.Weapons[(int)LaraWeaponType::Shotgun].Present == true)
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::Shotgun;

	if (KeyMap[DIK_3] && Lara.Weapons[(int)LaraWeaponType::Revolver].Present == true)
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::Revolver;

	if (KeyMap[DIK_4] && Lara.Weapons[(int)LaraWeaponType::Uzi].Present == true)
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::Uzi;

	if (KeyMap[DIK_5] && Lara.Weapons[(int)LaraWeaponType::HarpoonGun].Present == true)
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::HarpoonGun;

	if (KeyMap[DIK_6] && Lara.Weapons[(int)LaraWeaponType::HK].Present == true)
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::HK;

	if (KeyMap[DIK_7] && Lara.Weapons[(int)LaraWeaponType::RocketLauncher].Present == true)
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::RocketLauncher;

	if (KeyMap[DIK_8] && Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Present == true)
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::GrenadeLauncher;
	/*------------------------------------------------------------------*/

	if (KeyMap[DIK_0])
	{
		if (medipackTimeout == 0)
		{
			if (LaraItem->HitPoints > 0 && LaraItem->HitPoints < LARA_HEALTH_MAX || Lara.PoisonPotency)
			{
				if (Lara.Inventory.TotalSmallMedipacks != 0)
				{
					if (Lara.Inventory.TotalSmallMedipacks != -1)
						Lara.Inventory.TotalSmallMedipacks--;

					Lara.PoisonPotency = 0;
					LaraItem->HitPoints += LARA_HEALTH_MAX / 2;
					SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always); // TODO: Fix heal sound not triggering if small medi doesn't top off Lara's health. original tr4/5 issue

					if (LaraItem->HitPoints > LARA_HEALTH_MAX)
					{
						LaraItem->HitPoints = LARA_HEALTH_MAX;
						SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always);
						Statistics.Game.HealthUsed++;
					}
				}

				medipackTimeout = 15;
			}
		}
	}
	else if (KeyMap[DIK_9])
	{
		if (medipackTimeout == 0)
		{
			if (LaraItem->HitPoints > 0 && LaraItem->HitPoints < LARA_HEALTH_MAX || Lara.PoisonPotency)
			{
				if (Lara.Inventory.TotalLargeMedipacks != 0)
				{
					if (Lara.Inventory.TotalLargeMedipacks != -1)
						Lara.Inventory.TotalLargeMedipacks--;

					Lara.PoisonPotency = 0;
					LaraItem->HitPoints += LARA_HEALTH_MAX;

					if (LaraItem->HitPoints > LARA_HEALTH_MAX)
					{
						LaraItem->HitPoints = LARA_HEALTH_MAX;
						SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always);
						Statistics.Game.HealthUsed++;
					}
				}

				medipackTimeout = 15;
			}
		}
	}
	else if (medipackTimeout != 0)
		medipackTimeout--;

	if (KeyMap[DIK_F10])
		lInput |= IN_E;

	// TODO: Make FORWARD+BACK to turn 180 degrees a user option.
	/*if (lInput & IN_FORWARD && lInput & IN_BACK)
		lInput |= IN_ROLL;*/
	if (lInput & IN_ROLL && BinocularRange)
		lInput &= ~IN_ROLL;

	// Block simultaneous LEFT+RIGHT input.
	if (lInput & IN_LEFT && lInput & IN_RIGHT)
		lInput &= ~(IN_LEFT | IN_RIGHT);

	if (SetDebounce)
		DbInput = InputBusy;

	if (KeyMap[DIK_F5])
		lInput |= IN_SAVE;

	if (KeyMap[DIK_F6])
		lInput |= IN_LOAD;

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

	InputBusy = lInput;
	TrInput = lInput;

	if (Lara.Inventory.IsBusy)
	{
		lInput &= (IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_OPTION | IN_LOOK | IN_PAUSE);

		if (lInput & IN_FORWARD && lInput & IN_BACK)
			lInput &= ~IN_BACK;
	}

	if (SetDebounce)
		DbInput = TrInput & (DbInput ^ TrInput);

	if (KeyMap[DIK_F])
		lInput |= IN_FORWARD;
	else
		lInput &= ~IN_FORWARD;

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
