#include "config.h"

#include "winmain.h"
#include "..\resource.h"
#include "..\Renderer\Renderer11.h"
#include "..\Specific\input.h"

#include <CommCtrl.h>

extern Renderer11* g_Renderer;

GameConfiguration g_Configuration;

void __cdecl LoadResolutionsInCombobox(HWND handle, __int32 index)
{
	HWND cbHandle = GetDlgItem(handle, IDC_CB_MODES);

	SendMessageA(cbHandle, CB_RESETCONTENT, 0, 0);

	vector<RendererVideoAdapter>* adapters = g_Renderer->GetAdapters();
	RendererVideoAdapter* adapter = &(*adapters)[index];

	for (__int32 i = 0; i < adapter->DisplayModes.size(); i++)
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

void __cdecl LoadAdaptersInCombobox(HWND handle)
{
	HWND cbHandle = GetDlgItem(handle, IDC_CB_ADAPTERS);

	SendMessageA(cbHandle, CB_RESETCONTENT, 0, 0);

	vector<RendererVideoAdapter>* adapters = g_Renderer->GetAdapters();
	for (__int32 i = 0; i < adapters->size(); i++)
	{
		RendererVideoAdapter* adapter = &(*adapters)[i];
		SendMessageA(cbHandle, CB_ADDSTRING, i, (LPARAM)adapter->Name.c_str());
	}

	SendMessageA(cbHandle, CB_SETCURSEL, 0, 0);
	LoadResolutionsInCombobox(handle, 0);
}

BOOL CALLBACK DialogProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND ctlHandle;

	__int32 selectedIndex;
	RendererVideoAdapter* adapter;
	RendererDisplayMode* mode;

	switch (msg)
	{
	case WM_INITDIALOG:
		DB_Log(6, "WM_INITDIALOG");

		LoadAdaptersInCombobox(handle);

		// Set some default values
		g_Configuration.AutoTarget = true;
		SendDlgItemMessage(handle, IDC_CHK_AUTOTARGET, BM_SETCHECK, 1, 0);

		g_Configuration.EnableVolumetricFog = true;
		SendDlgItemMessage(handle, IDC_CHK_VOLUMETRIC_FOG, BM_SETCHECK, 1, 0);

		g_Configuration.EnableShadows = true;
		SendDlgItemMessage(handle, IDC_CHK_SHADOWS, BM_SETCHECK, 1, 0);

		g_Configuration.EnableCaustics = true;
		SendDlgItemMessage(handle, IDC_CHK_CAUSTICS, BM_SETCHECK, 1, 0);

		g_Configuration.Windowed = true;
		SendDlgItemMessage(handle, IDC_CHK_WINDOWED, BM_SETCHECK, 1, 0);

		g_Configuration.EnableSound = true;
		SendDlgItemMessage(handle, IDC_CHK_ENABLE_SOUND, BM_SETCHECK, 1, 0);

		g_Configuration.EnableAudioSpecialEffects = true;
		SendDlgItemMessage(handle, IDC_CHK_ENABLE_SOUND_SPECIAL_FX, BM_SETCHECK, 1, 0);

		g_Configuration.MusicVolume = 100;
		SendDlgItemMessage(handle, IDC_SL_MUSIC_VOLUME, TBM_SETPOS, true, 100);

		g_Configuration.SfxVolume = 100;
		SendDlgItemMessage(handle, IDC_SL_SFX_VOLUME, TBM_SETPOS, true, 100);

		break;

	case WM_HSCROLL:
		DB_Log(6, "WM_HSCROLL");

		if (lParam == (LPARAM)GetDlgItem(handle, IDC_SL_MUSIC_VOLUME))
		{
			g_Configuration.MusicVolume = (SendMessage((HWND)lParam, TBM_GETPOS, 0, 0));
			break;
		}
		else if (lParam == (LPARAM)GetDlgItem(handle, IDC_SL_SFX_VOLUME))
		{
			g_Configuration.SfxVolume = (SendMessage((HWND)lParam, TBM_GETPOS, 0, 0));
			break;
		}

		break;

	case WM_COMMAND:
		DB_Log(6, "WM_COMMAND");

		// Checkboxes
		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case IDC_CHK_WINDOWED:
				g_Configuration.Windowed = (SendDlgItemMessage(handle, IDC_CHK_WINDOWED, BM_GETCHECK, 0, 0));
				break;

			case IDC_CHK_SHADOWS:
				g_Configuration.EnableShadows = (SendDlgItemMessage(handle, IDC_CHK_SHADOWS, BM_GETCHECK, 0, 0));
				break;

			case IDC_CHK_CAUSTICS:
				g_Configuration.EnableCaustics = (SendDlgItemMessage(handle, IDC_CHK_CAUSTICS, BM_GETCHECK, 0, 0));
				break;

			case IDC_CHK_VOLUMETRIC_FOG:
				g_Configuration.EnableVolumetricFog = (SendDlgItemMessage(handle, IDC_CHK_VOLUMETRIC_FOG, BM_GETCHECK, 0, 0));
				break;

			case IDC_CHK_AUTOTARGET:
				g_Configuration.AutoTarget = (SendDlgItemMessage(handle, IDC_CHK_AUTOTARGET, BM_GETCHECK, 0, 0));
				break;

			case IDC_CHK_ENABLE_SOUND:
				g_Configuration.EnableSound = (SendDlgItemMessage(handle, IDC_CHK_ENABLE_SOUND, BM_GETCHECK, 0, 0));
				break;

			case IDC_CHK_ENABLE_SOUND_SPECIAL_FX:
				g_Configuration.EnableAudioSpecialEffects = (SendDlgItemMessage(handle, IDC_CHK_ENABLE_SOUND_SPECIAL_FX, BM_GETCHECK, 0, 0));
				break;
			
			case IDOK:
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
			case IDC_CB_ADAPTERS:
				g_Configuration.Adapter = (SendDlgItemMessage(handle, IDC_CB_ADAPTERS, CB_GETCURSEL, 0, 0));
				LoadResolutionsInCombobox(handle, g_Configuration.Adapter);
				break;

			case IDC_CB_MODES:
				selectedIndex = (SendDlgItemMessage(handle, IDC_CB_MODES, CB_GETCURSEL, 0, 0));
				adapter = &(*g_Renderer->GetAdapters())[g_Configuration.Adapter];
				mode = &(adapter->DisplayModes[selectedIndex]);
				
				g_Configuration.Width = mode->Width;
				g_Configuration.Height = mode->Height;
				g_Configuration.RefreshRate = mode->RefreshRate;

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

__int32 __cdecl SetupDialog()
{
	InitCommonControls();
	HRSRC res = FindResource(g_DllHandle, MAKEINTRESOURCE(IDD_SETUP_WINDOW), RT_DIALOG);

	ShowCursor(true);
	__int32 result = DialogBoxParamA(g_DllHandle, MAKEINTRESOURCE(IDD_SETUP_WINDOW), 0, (DLGPROC)DialogProc, 0);
	ShowCursor(false);

	return true;
}

bool __cdecl SaveConfiguration()
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

	if (SetDWORDRegKey(rootKey, REGKEY_REFRESH_RATE, g_Configuration.RefreshRate) != ERROR_SUCCESS)
	{
		RegCloseKey(rootKey);
		return false;
	}

	for (int i = 0; i < NUM_CONTROLS; i++)
	{
		char buffer[6];
		sprintf(buffer, "Key%d", i);

		if (SetDWORDRegKey(rootKey, buffer, KeyboardLayout1[i]) != ERROR_SUCCESS)
		{
			RegCloseKey(rootKey);
			return false;
		}
	}

	OptionAutoTarget = g_Configuration.AutoTarget;
	GlobalMusicVolume = g_Configuration.MusicVolume;
	GlobalFXVolume = g_Configuration.SfxVolume;
	
	return true;
}

bool __cdecl LoadConfiguration()
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
	if (GetBoolRegKey(rootKey, REGKEY_ENABLE_SOUND, &enableSound, false) != ERROR_SUCCESS)
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

	for (int i = 0; i < NUM_CONTROLS; i++)
	{
		DWORD tempKey;
		char buffer[6];
		sprintf(buffer, "Key%d", i);

		if (GetDWORDRegKey(rootKey, buffer, &tempKey, KeyboardLayout0[i]) != ERROR_SUCCESS)
		{
			RegCloseKey(rootKey);
			return false;
		}

		KeyboardLayout1[i] = (__int16)tempKey;
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

	// Set legacy variables
	OptionAutoTarget = autoTarget;
	GlobalMusicVolume = musicVolume;
	GlobalFXVolume = sfxVolume;

	RegCloseKey(rootKey);

	CheckKeyConflicts();

	return true;
}

LONG SetDWORDRegKey(HKEY hKey, char* strValueName, DWORD nValue)
{
	return RegSetValueExA(hKey, strValueName, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&nValue), sizeof(DWORD));
}

LONG SetBoolRegKey(HKEY hKey, char* strValueName, bool bValue)
{
	return SetDWORDRegKey(hKey, strValueName, (bValue ? 1 : 0));
}

LONG SetStringRegKey(HKEY hKey, char* strValueName, char* strValue)
{
	return 1; // RegSetValueExA(hKey, strValueName, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&nValue), sizeof(DWORD));
}

LONG GetDWORDRegKey(HKEY hKey, char* strValueName, DWORD* nValue, DWORD nDefaultValue)
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


LONG GetBoolRegKey(HKEY hKey, char* strValueName, bool* bValue, bool bDefaultValue)
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


LONG GetStringRegKey(HKEY hKey, char* strValueName, char** strValue, char* strDefaultValue)
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