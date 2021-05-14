#include "framework.h"
#include "configuration.h"
#include "winmain.h"
#include "resource.h"
#include "Renderer11.h"
#include "input.h"
#include "GameFlowScript.h"
#include "configuration.h"
#include "sound.h"
#include <CommCtrl.h>
using namespace T5M::Renderer;
using std::vector;
extern GameFlow* g_GameFlow;
GameConfiguration g_Configuration;

void LoadResolutionsInCombobox(HWND handle, int index)
{
	HWND cbHandle = GetDlgItem(handle, IDC_RESOLUTION);

	SendMessageA(cbHandle, CB_RESETCONTENT, 0, 0);

	vector<RendererVideoAdapter>* adapters = g_Renderer.getAdapters();
	RendererVideoAdapter* adapter = &(*adapters)[index];

	for (int i = 0; i < adapter->DisplayModes.size(); i++)
	{
		RendererDisplayMode* mode = &(adapter->DisplayModes)[i];

		char* str = (char*)malloc(255);
		ZeroMemory(str, 255);
		sprintf(str, "%d x %d (%d Hz)", mode->Width, mode->Height, mode->RefreshRate);

		SendMessageA(cbHandle, CB_ADDSTRING, i, (LPARAM)(str));

		free(str);
	}

	SendMessageA(cbHandle, CB_SETCURSEL, 0, 0);
	SendMessageA(cbHandle, CB_SETMINVISIBLE, 20, 0);
}

void LoadAdaptersInCombobox(HWND handle)
{
	HWND cbHandle = GetDlgItem(handle, IDC_GFXADAPTER);

	SendMessageA(cbHandle, CB_RESETCONTENT, 0, 0);

	vector<RendererVideoAdapter>* adapters = g_Renderer.getAdapters();
	for (int i = 0; i < adapters->size(); i++)
	{
		RendererVideoAdapter* adapter = &(*adapters)[i];
		SendMessageA(cbHandle, CB_ADDSTRING, i, (LPARAM)adapter->Name.c_str());
	}

	SendMessageA(cbHandle, CB_SETCURSEL, 0, 0);
	LoadResolutionsInCombobox(handle, 0);
}

void LoadSoundDevicesInCombobox(HWND handle)
{
	HWND cbHandle = GetDlgItem(handle, IDC_SNDADAPTER);

	SendMessageA(cbHandle, CB_RESETCONTENT, 0, 0);

	// Get all audio devices, including the default one
	BASS_DEVICEINFO info;
	int i = 1;
	while (BASS_GetDeviceInfo(i, &info))
	{
		SendMessageA(cbHandle, CB_ADDSTRING, 0, (LPARAM)info.name);
		i++;
	}

	SendMessageA(cbHandle, CB_SETCURSEL, 0, 0);
}

BOOL CALLBACK DialogProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND ctlHandle;

	RendererVideoAdapter* adapter;
	RendererDisplayMode* mode;
	int selectedAdapter;
	int selectedMode;

	switch (msg)
	{
	case WM_INITDIALOG:
		//DB_Log(6, "WM_INITDIALOG");

		SendMessageA(GetDlgItem(handle, IDC_GROUP_GFXADAPTER), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_DISPLAY_ADAPTER));
		SendMessageA(GetDlgItem(handle, IDC_GROUP_OUTPUT_SETTINGS), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_OUTPUT_SETTINGS));
		SendMessageA(GetDlgItem(handle, IDOK), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_OK));
		SendMessageA(GetDlgItem(handle, IDCANCEL), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_CANCEL));
		SendMessageA(GetDlgItem(handle, IDC_GROUP_RESOLUTION), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_SCREEN_RESOLUTION));
		SendMessageA(GetDlgItem(handle, IDC_GROUP_SOUND), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_SOUND));
		SendMessageA(GetDlgItem(handle, IDC_ENABLE_SOUNDS), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_ENABLE_SOUND));
		SendMessageA(GetDlgItem(handle, IDC_WINDOWED), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_WINDOWED));
		SendMessageA(GetDlgItem(handle, IDC_GROUP_RENDER_OPTIONS), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_RENDER_OPTIONS));
		SendMessageA(GetDlgItem(handle, IDC_SHADOWS), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_SHADOWS));
		SendMessageA(GetDlgItem(handle, IDC_CAUSTICS), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_CAUSTICS));
		SendMessageA(GetDlgItem(handle, IDC_VOLUMETRIC_FOG), WM_SETTEXT, 0, (LPARAM)g_GameFlow->GetString(STRING_VOLUMETRIC_FOG));

		LoadAdaptersInCombobox(handle);
		LoadSoundDevicesInCombobox(handle);

		// Set some default values
		g_Configuration.AutoTarget = true;

		g_Configuration.EnableVolumetricFog = true;
		SendDlgItemMessage(handle, IDC_VOLUMETRIC_FOG, BM_SETCHECK, 1, 0);

		g_Configuration.EnableShadows = true;
		SendDlgItemMessage(handle, IDC_SHADOWS, BM_SETCHECK, 1, 0);

		g_Configuration.EnableCaustics = true;
		SendDlgItemMessage(handle, IDC_CAUSTICS, BM_SETCHECK, 1, 0);

		g_Configuration.Windowed = true;
		SendDlgItemMessage(handle, IDC_WINDOWED, BM_SETCHECK, 1, 0);

		g_Configuration.EnableSound = true;
		SendDlgItemMessage(handle, IDC_ENABLE_SOUNDS, BM_SETCHECK, 1, 0);

		break;

	case WM_COMMAND:
		//DB_Log(6, "WM_COMMAND");

		// Checkboxes
		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				// Get values from dialog components
				g_Configuration.Windowed = (SendDlgItemMessage(handle, IDC_WINDOWED, BM_GETCHECK, 0, 0));
				g_Configuration.EnableShadows = (SendDlgItemMessage(handle, IDC_SHADOWS, BM_GETCHECK, 0, 0));
				g_Configuration.EnableCaustics = (SendDlgItemMessage(handle, IDC_CAUSTICS, BM_GETCHECK, 0, 0));
				g_Configuration.EnableVolumetricFog = (SendDlgItemMessage(handle, IDC_VOLUMETRIC_FOG, BM_GETCHECK, 0, 0));
				g_Configuration.EnableSound = (SendDlgItemMessage(handle, IDC_ENABLE_SOUNDS, BM_GETCHECK, 0, 0));
				g_Configuration.Adapter = (SendDlgItemMessage(handle, IDC_GFXADAPTER, CB_GETCURSEL, 0, 0));
				selectedMode = (SendDlgItemMessage(handle, IDC_RESOLUTION, CB_GETCURSEL, 0, 0));
				adapter = &(*g_Renderer.getAdapters())[g_Configuration.Adapter];
				mode = &(adapter->DisplayModes[selectedMode]);
				g_Configuration.Width = mode->Width;
				g_Configuration.Height = mode->Height;
				g_Configuration.RefreshRate = mode->RefreshRate;
				g_Configuration.SoundDevice = (SendDlgItemMessage(handle, IDC_SNDADAPTER, CB_GETCURSEL, 0, 0)) + 1;

				// Save the configuration
				SaveConfiguration();
				EndDialog(handle, wParam);
				return 1;

			case IDCANCEL:
				EndDialog(handle, wParam);
				return 1;

			}

			return 0;
		}

		// Comboboxes
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			switch (LOWORD(wParam))
			{
			case IDC_GFXADAPTER:
				selectedAdapter = (SendDlgItemMessage(handle, IDC_GFXADAPTER, CB_GETCURSEL, 0, 0));
				LoadResolutionsInCombobox(handle, selectedAdapter);
				break;
			}

			return 0;
		}

		break;

	default:
		return 0;
	}

	return 0;
}

int SetupDialog()
{
	// FIXME
	/*HRSRC res = FindResource(g_DllHandle, MAKEINTRESOURCE(IDD_SETUP), RT_DIALOG);

	ShowCursor(true);
	int result = DialogBoxParamA(g_DllHandle, MAKEINTRESOURCE(IDD_SETUP), 0, (DLGPROC)DialogProc, 0);
	ShowCursor(false);*/

	return true;
}

bool SaveConfiguration()
{
	// Try to open the root key
	HKEY rootKey = NULL;
	if (RegOpenKeyA(HKEY_CURRENT_USER, REGKEY_ROOT, &rootKey) != ERROR_SUCCESS)
	{
		// Create the new key
		if (RegCreateKeyA(HKEY_CURRENT_USER, REGKEY_ROOT, &rootKey) != ERROR_SUCCESS)
			return false;
	}

	if (SetDWORDRegKey(rootKey, REGKEY_ADAPTER, g_Configuration.Adapter) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetDWORDRegKey(rootKey, REGKEY_SCREEN_WIDTH, g_Configuration.Width) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetDWORDRegKey(rootKey, REGKEY_SCREEN_HEIGHT, g_Configuration.Height) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetBoolRegKey(rootKey, REGKEY_WINDOWED, g_Configuration.Windowed) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetBoolRegKey(rootKey, REGKEY_SHADOWS, g_Configuration.EnableShadows) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetBoolRegKey(rootKey, REGKEY_CAUSTICS, g_Configuration.EnableCaustics) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetBoolRegKey(rootKey, REGKEY_VOLUMETRIC_FOG, g_Configuration.EnableVolumetricFog) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetBoolRegKey(rootKey, REGKEY_AUTOTARGET, g_Configuration.AutoTarget) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetBoolRegKey(rootKey, REGKEY_ENABLE_SOUND, g_Configuration.EnableSound) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetDWORDRegKey(rootKey, REGKEY_SOUND_DEVICE, g_Configuration.SoundDevice) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetBoolRegKey(rootKey, REGKEY_SOUND_SPECIAL_FX, g_Configuration.EnableAudioSpecialEffects) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetDWORDRegKey(rootKey, REGKEY_MUSIC_VOLUME, g_Configuration.MusicVolume) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if (SetDWORDRegKey(rootKey, REGKEY_SFX_VOLUME, g_Configuration.SfxVolume) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	if(SetDWORDRegKey(rootKey, REGKEY_REFRESH_RATE, g_Configuration.RefreshRate) != ERROR_SUCCESS){
		RegCloseKey(rootKey);
		return false;
	}

	if(SetDWORDRegKey(rootKey, REGKEY_SHADOW_MAP, g_Configuration.shadowMapSize) != ERROR_SUCCESS){
		RegCloseKey(rootKey);
		return false;
	}

	for (int i = 0; i < NUM_CONTROLS; i++)
	{
		char buffer[6];
		sprintf(buffer, "Key%d", i);

		if (SetDWORDRegKey(rootKey, buffer, KeyboardLayout[1][i]) != ERROR_SUCCESS)
		{
			RegCloseKey(rootKey);
			return false;
		}
	}

	return true;
}

void SaveAudioConfig()
{
	GlobalMusicVolume = g_Configuration.MusicVolume;
	GlobalFXVolume = g_Configuration.SfxVolume;
}

void InitDefaultConfiguration()
{
	// Include default device into the list
	BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, true);

	g_Configuration.AutoTarget = true;
	g_Configuration.SoundDevice = 1;
	g_Configuration.Adapter = 0;
	g_Configuration.EnableAudioSpecialEffects = true;
	g_Configuration.EnableCaustics = true;
	g_Configuration.EnableShadows = true;
	g_Configuration.EnableSound = true;
	g_Configuration.EnableVolumetricFog = true;
	g_Configuration.MusicVolume = 100;
	g_Configuration.SfxVolume = 100;
	g_Configuration.Width = 1366;
	g_Configuration.Height = 768;
	g_Configuration.shadowMapSize = 512;
}

bool LoadConfiguration()
{
	// Try to open the root key
	HKEY rootKey = NULL;
	if (RegOpenKeyA(HKEY_CURRENT_USER, REGKEY_ROOT, &rootKey) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	// Load configuration keys
	DWORD adapter = 0;
	if (GetDWORDRegKey(rootKey, REGKEY_ADAPTER, &adapter, 0) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	DWORD screenWidth = 0;
	if (GetDWORDRegKey(rootKey, REGKEY_SCREEN_WIDTH, &screenWidth, 0) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	DWORD screenHeight = 0;
	if (GetDWORDRegKey(rootKey, REGKEY_SCREEN_HEIGHT, &screenHeight, 0) != ERROR_SUCCESS)
		return false;

	DWORD refreshRate = 0;
	if (GetDWORDRegKey(rootKey, REGKEY_REFRESH_RATE, &refreshRate, 0) != ERROR_SUCCESS)
		return false;

	bool windowed = false;
	if (GetBoolRegKey(rootKey, REGKEY_WINDOWED, &windowed, false) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	bool caustics = false;
	if (GetBoolRegKey(rootKey, REGKEY_CAUSTICS, &caustics, true) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	bool volumetricFog = false;
	if (GetBoolRegKey(rootKey, REGKEY_VOLUMETRIC_FOG, &volumetricFog, true) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	bool shadows = false;
	if (GetBoolRegKey(rootKey, REGKEY_SHADOWS, &shadows, true) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	bool autoTarget = false;
	if (GetBoolRegKey(rootKey, REGKEY_AUTOTARGET, &autoTarget, true) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	bool enableSound = false;
	if (GetBoolRegKey(rootKey, REGKEY_ENABLE_SOUND, &enableSound, true) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	bool enableSoundSpecialEffects = false;
	if (GetBoolRegKey(rootKey, REGKEY_SOUND_SPECIAL_FX, &enableSoundSpecialEffects, false) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	DWORD musicVolume = 100;
	if (GetDWORDRegKey(rootKey, REGKEY_MUSIC_VOLUME, &musicVolume, 100) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	DWORD sfxVolume = 100;
	if (GetDWORDRegKey(rootKey, REGKEY_SFX_VOLUME, &sfxVolume, 100) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	DWORD soundDevice = 0;
	if (GetDWORDRegKey(rootKey, REGKEY_SOUND_DEVICE, &soundDevice, 1) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	for (int i = 0; i < NUM_CONTROLS; i++)
	{
		DWORD tempKey;
		char buffer[6];
		sprintf(buffer, "Key%d", i);

		if (GetDWORDRegKey(rootKey, buffer, &tempKey, KeyboardLayout[0][i]) != ERROR_SUCCESS)
		{
			RegCloseKey(rootKey);
			return false;
		}

		KeyboardLayout[1][i] = (short)tempKey;
	}

	// All configuration values were found, so I can apply configuration to the engine
	g_Configuration.AutoTarget = autoTarget;
	g_Configuration.Width = screenWidth;
	g_Configuration.Height = screenHeight;
	g_Configuration.Windowed = windowed;
	g_Configuration.Adapter = adapter;
	g_Configuration.EnableShadows = shadows;
	g_Configuration.EnableCaustics = caustics;
	g_Configuration.EnableVolumetricFog = volumetricFog;
	g_Configuration.EnableSound = enableSound;
	g_Configuration.EnableAudioSpecialEffects = enableSoundSpecialEffects;
	g_Configuration.MusicVolume = musicVolume;
	g_Configuration.SfxVolume = sfxVolume;
	g_Configuration.RefreshRate = refreshRate;
	g_Configuration.SoundDevice = soundDevice;
	g_Configuration.shadowMapSize = 512;
	// Set legacy variables
	//OptionAutoTarget = autoTarget;
	GlobalMusicVolume = musicVolume;
	GlobalFXVolume = sfxVolume;

	RegCloseKey(rootKey);

	DefaultConflict();

	return true;
}

LONG SetDWORDRegKey(HKEY hKey, LPCSTR strValueName, DWORD nValue)
{
	return RegSetValueExA(hKey, strValueName, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&nValue), sizeof(DWORD));
}

LONG SetBoolRegKey(HKEY hKey, LPCSTR strValueName, bool bValue)
{
	return SetDWORDRegKey(hKey, strValueName, (bValue ? 1 : 0));
}

LONG SetStringRegKey(HKEY hKey, LPCSTR strValueName, char* strValue)
{
	return 1; // RegSetValueExA(hKey, strValueName, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&nValue), sizeof(DWORD));
}

LONG GetDWORDRegKey(HKEY hKey, LPCSTR strValueName, DWORD* nValue, DWORD nDefaultValue)
{
	*nValue = nDefaultValue;
	DWORD dwBufferSize(sizeof(DWORD));
	DWORD nResult(0);
	LONG nError = ::RegQueryValueEx(hKey,
		strValueName,
		0,
		NULL,
		reinterpret_cast<LPBYTE>(&nResult),
		&dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		*nValue = nResult;
	}
	return nError;
}


LONG GetBoolRegKey(HKEY hKey, LPCSTR strValueName, bool* bValue, bool bDefaultValue)
{
	DWORD nDefValue((bDefaultValue) ? 1 : 0);
	DWORD nResult(nDefValue);
	LONG nError = GetDWORDRegKey(hKey, strValueName, &nResult, nDefValue);
	if (ERROR_SUCCESS == nError)
	{
		*bValue = (nResult != 0) ? true : false;
	}
	return nError;
}


LONG GetStringRegKey(HKEY hKey, LPCSTR strValueName, char** strValue, char* strDefaultValue)
{
	*strValue = strDefaultValue;
	char szBuffer[512];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG nError;
	nError = RegQueryValueEx(hKey, strValueName, 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		*strValue = szBuffer;
	}
	return nError;
}