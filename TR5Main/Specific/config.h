#pragma once

#include <Windows.h>

#define REGKEY_ROOT				"Software\\TR5Main\\TR5Main"

#define REGKEY_ADAPTER			"Adapter"
#define REGKEY_SCREEN_WIDTH		"ScreenWidth"
#define REGKEY_SCREEN_HEIGHT	"ScreenHeight"
#define REGKEY_WINDOWED			"Windowed"
#define REGKEY_DISABLE_SOUND	"DisableSound"
#define REGKEY_SHADOWS			"Shadows"
#define REGKEY_CAUSTICS			"Caustics"
#define REGKEY_VOLUMETRIC_FOG	"VolumetricFog"
#define REGKEY_AUTOTARGET		"AutoTarget"

typedef struct GameConfiguration {
	__int32 Width;
	__int32 Height;
	__int32 Adapter;
	bool Windowed;
	bool DisableSound;
	bool AutoTarget;
	bool EnableShadows;
	bool EnableCaustics;
	bool EnableVolumetricFog;
};

void __cdecl LoadResolutionsInCombobox(HWND handle, __int32 index);
void __cdecl LoadAdaptersInCombobox(HWND handle);
BOOL CALLBACK DialogProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam);
__int32 __cdecl SetupDialog();
bool __cdecl LoadConfiguration();
bool __cdecl SaveConfiguration();
bool __cdecl FileExists(char* fileName);
LONG GetDWORDRegKey(HKEY hKey, char* strValueName, DWORD* nValue, DWORD nDefaultValue);
LONG GetBoolRegKey(HKEY hKey, char* strValueName, bool* bValue, bool bDefaultValue);
LONG GetStringRegKey(HKEY hKey, char* strValueName, char** strValue, char* strDefaultValue);
LONG SetDWORDRegKey(HKEY hKey, char* strValueName, DWORD nValue);
LONG SetBoolRegKey(HKEY hKey, char* strValueName, bool bValue);
LONG SetStringRegKey(HKEY hKey, char* strValueName, char* strValue);

extern GameConfiguration g_Configuration;