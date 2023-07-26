#pragma once
#include "Math/Math.h"
#include "Specific/Input/Input.h"
#include "Renderer/Renderer11Enums.h"

using namespace TEN::Input;
using namespace TEN::Math;

// Directories
constexpr auto REGKEY_ROOT	   = "Software\\TombEngine\\1.1.0";
constexpr auto REGKEY_GRAPHICS = "Graphics";
constexpr auto REGKEY_SOUND	   = "Sound";
constexpr auto REGKEY_GAMEPLAY = "Gameplay";
constexpr auto REGKEY_INPUT	   = "Input";

// Graphics keys
constexpr auto REGKEY_SCREEN_WIDTH		   = "ScreenWidth";
constexpr auto REGKEY_SCREEN_HEIGHT		   = "ScreenHeight";
constexpr auto REGKEY_ENABLE_WINDOWED_MODE = "EnableWindowedMode";
constexpr auto REGKEY_SHADOWS			   = "ShadowsMode";
constexpr auto REGKEY_SHADOW_MAP_SIZE	   = "ShadowMapSize";
constexpr auto REGKEY_SHADOW_BLOBS_MAX	   = "ShadowBlobsMax";
constexpr auto REGKEY_ENABLE_CAUSTICS	   = "EnableCaustics";
constexpr auto REGKEY_ANTIALIASING_MODE	   = "AntialiasingMode";

// Sound keys
constexpr auto REGKEY_SOUND_DEVICE	= "SoundDevice";
constexpr auto REGKEY_ENABLE_SOUND	= "EnableSound";
constexpr auto REGKEY_ENABLE_REVERB = "EnableReverb";
constexpr auto REGKEY_MUSIC_VOLUME	= "MusicVolume";
constexpr auto REGKEY_SFX_VOLUME	= "SfxVolume";

// Gameplay keys
constexpr auto REGKEY_ENABLE_SUBTITLES		   = "EnableSubtitles";
constexpr auto REGKEY_ENABLE_AUTO_TARGETING	   = "EnableAutoTargeting";
constexpr auto REGKEY_ENABLE_RUMBLE			   = "EnableRumble";
constexpr auto REGKEY_ENABLE_THUMBSTICK_CAMERA = "EnableThumbstickCamera";

struct GameConfiguration 
{
	// Graphics
	int		   ScreenWidth		  = 0;
	int		   ScreenHeight		  = 0;
	bool	   EnableWindowedMode = false;
	ShadowMode ShadowType		  = ShadowMode::None;
	int		   ShadowMapSize	  = 1024;
	int		   ShadowBlobsMax	  = 16;
	bool	   EnableCaustics	  = false;
	AntialiasingMode AntialiasingMode = AntialiasingMode::None;

	// Sound
	int	 SoundDevice  = 0;
	bool EnableSound  = false;
	bool EnableReverb = false;
	int	 MusicVolume  = 0;
	int	 SfxVolume	  = 0;

	// Gaeplay
	bool EnableSubtitles		= false;
	bool EnableAutoTargeting	= false;
	bool EnableRumble			= false;
	bool EnableThumbstickCamera = false;

	// Input
	std::vector<int> Bindings = {};

	std::vector<Vector2i> SupportedScreenResolutions = {};
	std::string			  AdapterName				 = {};
};

void LoadResolutionsInCombobox(HWND handle);
void LoadSoundDevicesInCombobox(HWND handle);
BOOL CALLBACK DialogProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam);
int	 SetupDialog();
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
