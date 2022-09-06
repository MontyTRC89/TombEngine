#pragma once

#include "Specific/input.h"
#include "Specific/trmath.h"
#include "Renderer/Renderer11Enums.h"

#define REGKEY_ROOT						"Software\\TombEngine\\TombEngine"

#define REGKEY_SCREEN_WIDTH				"ScreenWidth"
#define REGKEY_SCREEN_HEIGHT			"ScreenHeight"
#define REGKEY_WINDOWED					"Windowed"
#define REGKEY_SHADOWS					"Shadows"
#define REGKEY_SHADOW_MAP				"ShadowMap"
#define REGKEY_SHADOW_BLOBS				"ShadowBlobs"
#define REGKEY_CAUSTICS					"Caustics"
#define REGKEY_ANTIALIASING				"Antialiasing"

#define REGKEY_SOUND_DEVICE				"SoundDevice"
#define REGKEY_ENABLE_SOUND				"EnableSound"
#define REGKEY_SOUND_SPECIAL_FX			"EnableReverb"
#define REGKEY_SFX_VOLUME				"SfxVolume"
#define REGKEY_MUSIC_VOLUME				"MusicVolume"

#define REGKEY_ENABLE_RUMBLE			"EnableRumble"
#define REGKEY_ENABLE_THUMBSTICK_CAMERA	"EnableThumbstickCamera"

#define REGKEY_AUTOTARGET				"AutoTarget"

struct GameConfiguration 
{
	int Width;
	int Height;
	bool Windowed;

	bool EnableSound;
	bool EnableReverb;
	int SoundDevice;
	int MusicVolume;
	int SfxVolume;

	bool EnableCaustics;
	AntialiasingMode Antialiasing;
	ShadowMode ShadowType;
	int ShadowMapSize = 1024;
	int ShadowMaxBlobs = 16;

	bool AutoTarget;
	bool EnableRumble;
	bool EnableThumbstickCameraControl;
	short KeyboardLayout[TEN::Input::KEY_COUNT];

	std::vector<Vector2i> SupportedScreenResolutions;
	std::string AdapterName;
};

void LoadResolutionsInCombobox(HWND handle);
void LoadSoundDevicesInCombobox(HWND handle);
BOOL CALLBACK DialogProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam);
int SetupDialog();
void InitDefaultConfiguration();
bool LoadConfiguration();
bool SaveConfiguration();
void SaveAudioConfig();
LONG GetDWORDRegKey(HKEY hKey, LPCSTR strValueName, DWORD* nValue, DWORD nDefaultValue);
LONG GetBoolRegKey(HKEY hKey, LPCSTR strValueName, bool* bValue, bool bDefaultValue);
LONG GetStringRegKey(HKEY hKey, LPCSTR strValueName, char** strValue, char* strDefaultValue);
LONG SetDWORDRegKey(HKEY hKey, LPCSTR strValueName, DWORD nValue);
LONG SetBoolRegKey(HKEY hKey, LPCSTR strValueName, bool bValue);
LONG SetStringRegKey(HKEY hKey, LPCSTR strValueName, char* strValue);

extern GameConfiguration g_Configuration;