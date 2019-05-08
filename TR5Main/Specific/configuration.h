#pragma once

#include <Windows.h>
#include "input.h"

#define REGKEY_ROOT				"Software\\TR5Main\\TR5Main"

#define REGKEY_ADAPTER			"Adapter"
#define REGKEY_SCREEN_WIDTH		"ScreenWidth"
#define REGKEY_SCREEN_HEIGHT	"ScreenHeight"
#define REGKEY_WINDOWED			"Windowed"
#define REGKEY_ENABLE_SOUND		"EnableSound"
#define REGKEY_SOUND_SPECIAL_FX	"EnableSoundSpecialEffects"
#define REGKEY_SHADOWS			"Shadows"
#define REGKEY_CAUSTICS			"Caustics"
#define REGKEY_VOLUMETRIC_FOG	"VolumetricFog"
#define REGKEY_AUTOTARGET		"AutoTarget"
#define REGKEY_MUSIC_VOLUME		"MusicVolume"
#define REGKEY_SFX_VOLUME		"SfxVolume"
#define REGKEY_KEY0				"Key0"
#define REGKEY_KEY1				"Key1"
#define REGKEY_KEY2				"Key2"
#define REGKEY_KEY3				"Key3"
#define REGKEY_KEY4				"Key4"
#define REGKEY_KEY5				"Key5"
#define REGKEY_KEY6				"Key6"
#define REGKEY_KEY7				"Key7"
#define REGKEY_KEY8				"Key8"
#define REGKEY_KEY9				"Key9"
#define REGKEY_KEY10			"Key10"
#define REGKEY_KEY11			"Key11"
#define REGKEY_KEY12			"Key12"
#define REGKEY_KEY13			"Key13"
#define REGKEY_KEY14			"Key14"
#define REGKEY_KEY15			"Key15"
#define REGKEY_KEY16			"Key16"
#define REGKEY_KEY17			"Key17"
#define REGKEY_CONTROL_METHOD	"ControlMethod"
#define REGKEY_JDCK				"JDck"
#define REGKEY_JACT				"JAct"
#define REGKEY_JDRW				"JDrw"
#define REGKEY_JFLR				"JFlr"
#define REGKEY_JDSH				"JDsh"
#define REGKEY_JINV				"JInv"
#define REGKEY_JJMP				"JJmp"
#define REGKEY_JLOK				"JLok"
#define REGKEY_JROL				"JRol"
#define REGKEY_JWLK				"JWlk"
#define REGKEY_REFRESH_RATE		"RefreshRate"

typedef struct GameConfiguration {
	__int32 Width;
	__int32 Height;
	__int32 RefreshRate;
	__int32 Adapter;
	bool Windowed;
	bool EnableSound;
	bool AutoTarget;
	bool EnableShadows;
	bool EnableCaustics;
	bool EnableVolumetricFog;
	bool EnableAudioSpecialEffects;
	__int32 MaxDrawDistance;
	__int32 MusicVolume;
	__int32 SfxVolume;
	byte KeyboardLayout[NUM_CONTROLS];
	bool ControlMethod;
	__int32 JoyDuck;
	__int32 JoyAction;
	__int32 JoyDraw;
	__int32 JoyFlare;
	__int32 JoyDash;
	__int32 JoyInventory;
	__int32 JoyJump;
	__int32 JoyLook;
	__int32 JoyRoll;
	__int32 JoyWalk;
};

void __cdecl LoadResolutionsInCombobox(HWND handle, __int32 index);
void __cdecl LoadAdaptersInCombobox(HWND handle);
BOOL CALLBACK DialogProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam);
__int32 __cdecl SetupDialog();
bool __cdecl LoadConfiguration();
bool __cdecl SaveConfiguration();
LONG GetDWORDRegKey(HKEY hKey, char* strValueName, DWORD* nValue, DWORD nDefaultValue);
LONG GetBoolRegKey(HKEY hKey, char* strValueName, bool* bValue, bool bDefaultValue);
LONG GetStringRegKey(HKEY hKey, char* strValueName, char** strValue, char* strDefaultValue);
LONG SetDWORDRegKey(HKEY hKey, char* strValueName, DWORD nValue);
LONG SetBoolRegKey(HKEY hKey, char* strValueName, bool bValue);
LONG SetStringRegKey(HKEY hKey, char* strValueName, char* strValue);

extern GameConfiguration g_Configuration;