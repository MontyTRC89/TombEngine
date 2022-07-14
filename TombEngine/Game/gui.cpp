#include "framework.h"
#include "Game/gui.h"

#include <OISKeyboard.h>

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/pickup/pickup.h"
#include "Game/savegame.h"
#include "Game/spotcam.h"
#include "Renderer/Renderer11.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"
#include "Scripting/Include/ScriptInterfaceGame.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/configuration.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/configuration.h"

using namespace TEN::Input;
using namespace TEN::Renderer;

constexpr int LINE_HEIGHT = 25;
constexpr int PHD_CENTER_X = REFERENCE_RES_WIDTH / 2;
constexpr int PHD_CENTER_Y = REFERENCE_RES_HEIGHT / 2;

constexpr int VOLUME_MAX = 100;

GuiController g_Gui;

const char* OptionStrings[] =
{
	STRING_USE,
	STRING_CHOOSE_AMMO,
	STRING_COMBINE,
	STRING_SEPARE,
	STRING_EQUIP,
	STRING_COMBINE_WITH,
	STRING_LOAD_GAME,
	STRING_SAVE_GAME,
	STRING_EXAMINE,
	STRING_STATISTICS,
	STRING_CHOOSE_WEAPON,
	""
//	STRING_READ_DIARY
};

const char* ControlStrings[] =
{
	STRING_CONTROLS_MOVE_FORWARD,
	STRING_CONTROLS_MOVE_BACKWARD,
	STRING_CONTROLS_MOVE_LEFT,
	STRING_CONTROLS_MOVE_RIGHT,
	STRING_CONTROLS_CROUCH,
	STRING_CONTROLS_SPRINT,
	STRING_CONTROLS_WALK,
	STRING_CONTROLS_JUMP,
	STRING_CONTROLS_ACTION,
	STRING_CONTROLS_DRAW_WEAPON,
	STRING_CONTROLS_USE_FLARE,
	STRING_CONTROLS_LOOK,
	STRING_CONTROLS_ROLL,
	STRING_CONTROLS_INVENTORY,
	STRING_CONTROLS_PAUSE,
	STRING_CONTROLS_STEP_LEFT,
	STRING_CONTROLS_STEP_RIGHT,
	STRING_CONTROLS_ACCELERATE,
	STRING_CONTROLS_REVERSE,
	STRING_CONTROLS_SPEED,
	STRING_CONTROLS_SLOW,
	STRING_CONTROLS_BRAKE,
	STRING_CONTROLS_FIRE
};

SettingsData GuiController::GetCurrentSettings()
{
	return CurrentSettings;
}

InventoryRing* GuiController::GetRings(int ringIndex)
{
	return Rings[ringIndex];
}

short GuiController::GetSelectedOption()
{
	return SelectedOption;
}

Menu GuiController::GetMenuToDisplay()
{
	return MenuToDisplay;
}

void GuiController::SetSelectedOption(int menu)
{
	SelectedOption = menu;
}

void GuiController::SetMenuToDisplay(Menu menu)
{
	MenuToDisplay = menu;
}

InventoryMode GuiController::GetInventoryMode()
{
	return InvMode;
}

void GuiController::SetInventoryMode(InventoryMode mode)
{
	InvMode = mode;
}

void GuiController::SetInventoryItemChosen(int number)
{
	InventoryItemChosen = number;
}

int GuiController::GetInventoryItemChosen()
{
	return InventoryItemChosen;
}

void GuiController::SetEnterInventory(int number)
{
	EnterInventory = number;
}

int GuiController::GetEnterInventory()
{
	return EnterInventory;
}

int GuiController::GetLastInventoryItem()
{
	return LastInvItem;
}

void GuiController::DrawInventory()
{
	g_Renderer.RenderInventory();
}

InventoryResult GuiController::TitleOptions()
{
	auto inventoryResult = InventoryResult::None;
	static short selectedOptionBackup;

	// Stuff for credits goes here!

	switch (MenuToDisplay)
	{
	case Menu::Title:
		OptionCount = 3;
		break;

	case Menu::SelectLevel:
		inventoryResult = InventoryResult::None;
		OptionCount = g_GameFlow->GetNumLevels() - 2;
		break;

	case Menu::LoadGame:
		OptionCount = SAVEGAME_MAX - 1;
		break;

	case Menu::Options:
		OptionCount = 2;
		break;

	case Menu::Display:
		HandleDisplaySettingsInput(false);
		break;

	case Menu::Controls:
		HandleControlSettingsInput(false);
		break;

	case Menu::OtherSettings:
		HandleOtherSettingsInput(false);
		break;
	}

	if (MenuToDisplay == Menu::LoadGame)
	{
		switch (DoLoad())
		{
		case LoadResult::Load:
			inventoryResult = InventoryResult::LoadGame;
			break;

		case LoadResult::Cancel:
			MenuToDisplay = Menu::Title;
			SelectedOption = selectedOptionBackup;
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			break;

		case LoadResult::None:
			break;
		}
	}
	else if (MenuToDisplay == Menu::Title || 
		MenuToDisplay == Menu::SelectLevel || 
		MenuToDisplay == Menu::Options)
	{
		if (GUI_INPUT_PULSE_UP)
		{
			SelectedOption = (SelectedOption <= 0) ? OptionCount : (SelectedOption - 1);
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}

		if (GUI_INPUT_PULSE_DOWN)
		{
			if (SelectedOption < OptionCount)
				SelectedOption++;
			else
				SelectedOption -= OptionCount;

			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}

		if (GUI_INPUT_DESELECT &&
			MenuToDisplay != Menu::Title)
		{
			MenuToDisplay = Menu::Title;
			SelectedOption = selectedOptionBackup;
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
		}

		if (GUI_INPUT_SELECT)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

			if (MenuToDisplay == Menu::Title)
			{
				switch (SelectedOption)
				{
				case 0:
					if (g_GameFlow->CanPlayAnyLevel())
					{
						selectedOptionBackup = SelectedOption;
						SelectedOption = 0;
						MenuToDisplay = Menu::SelectLevel;
					}
					else
						inventoryResult = InventoryResult::NewGame;

					break;

				case 1:
					selectedOptionBackup = SelectedOption;
					SelectedOption = 0;
					MenuToDisplay = Menu::LoadGame;
					break;

				case 2:
					selectedOptionBackup = SelectedOption;
					SelectedOption = 0;
					MenuToDisplay = Menu::Options;
					break;

				case 3:
					inventoryResult = InventoryResult::ExitGame;
					break;
				}
			}
			else if (MenuToDisplay == Menu::SelectLevel)
			{
				// Level 0 is the title level, so increment the option by 1 to offset it.
				g_GameFlow->SelectedLevelForNewGame = SelectedOption + 1;
				MenuToDisplay = Menu::Title;
				SelectedOption = 0;
				inventoryResult = InventoryResult::NewGameSelectedLevel;
			}
			else if (MenuToDisplay == Menu::Options)
				HandleOptionsInput();
		}
	}

	return inventoryResult;
}

void GuiController::FillDisplayOptions()
{
	// Copy configuration to a temporary object
	BackupOptions();

	// Get current display mode
	CurrentSettings.SelectedScreenResolution = 0;
	for (int i = 0; i < g_Configuration.SupportedScreenResolutions.size(); i++)
	{
		auto screenResolution = g_Configuration.SupportedScreenResolutions[i];
		if (screenResolution.x == CurrentSettings.Configuration.Width &&
			screenResolution.y == CurrentSettings.Configuration.Height)
		{
			CurrentSettings.SelectedScreenResolution = i;
			break;
		}
	}
}

void GuiController::HandleDisplaySettingsInput(bool fromPauseMenu)
{
	OptionCount = 6;

	if (GUI_INPUT_DESELECT)
	{
		SoundEffect(SFX_TR4_MENU_SELECT, nullptr);
		MenuToDisplay = Menu::Options;
		SelectedOption = 0;
		return;
	}

	if (GUI_INPUT_PULSE_LEFT)
	{
		switch (SelectedOption)
		{
		case 0:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			if (CurrentSettings.SelectedScreenResolution > 0)
				CurrentSettings.SelectedScreenResolution--;

			break;

		case 1:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.Windowed = !CurrentSettings.Configuration.Windowed;
			break;

		case 2:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.ShadowMode--;
			if (CurrentSettings.Configuration.ShadowMode < SHADOW_NONE) CurrentSettings.Configuration.ShadowMode = SHADOW_ALL;
			break;

		case 3:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.EnableCaustics = !CurrentSettings.Configuration.EnableCaustics;
			break;

		case 4:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.EnableVolumetricFog = !CurrentSettings.Configuration.EnableVolumetricFog;
			break;
		}
	}

	if (GUI_INPUT_PULSE_RIGHT)
	{
		switch (SelectedOption)
		{
		case 0:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			if (CurrentSettings.SelectedScreenResolution < g_Configuration.SupportedScreenResolutions.size() - 1)
				CurrentSettings.SelectedScreenResolution++;

			break;

		case 1:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.Windowed = !CurrentSettings.Configuration.Windowed;
			break;

		case 2:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.ShadowMode++;
			if (CurrentSettings.Configuration.ShadowMode > SHADOW_ALL)
				CurrentSettings.Configuration.ShadowMode = SHADOW_NONE;

			break;

		case 3:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.EnableCaustics = !CurrentSettings.Configuration.EnableCaustics;
			break;

		case 4:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.EnableVolumetricFog = !CurrentSettings.Configuration.EnableVolumetricFog;
			break;
		}
	}

	if (GUI_INPUT_PULSE_UP)
	{
		if (SelectedOption <= 0)
			SelectedOption += OptionCount;
		else
			SelectedOption--;

		SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
	}

	if (GUI_INPUT_PULSE_DOWN)
	{
		if (SelectedOption < OptionCount)
			SelectedOption++;
		else
			SelectedOption -= OptionCount;

		SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
	}

	if (GUI_INPUT_SELECT)
	{
		SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

		if (SelectedOption == 5)
		{
			// Save the configuration
			auto screenResolution = g_Configuration.SupportedScreenResolutions[CurrentSettings.SelectedScreenResolution];
			CurrentSettings.Configuration.Width = screenResolution.x;
			CurrentSettings.Configuration.Height = screenResolution.y;

			memcpy(&g_Configuration, &CurrentSettings.Configuration, sizeof(GameConfiguration));
			SaveConfiguration();

			// Reset screen and go back
			g_Renderer.ChangeScreenResolution(CurrentSettings.Configuration.Width, CurrentSettings.Configuration.Height, 
				CurrentSettings.Configuration.Windowed);

			MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
			SelectedOption = fromPauseMenu ? 1 : 0;
		}
		else if (SelectedOption == 6)
		{
			MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
			SelectedOption = fromPauseMenu ? 1 : 0;
		}
	}
}

void GuiController::HandleControlSettingsInput(bool fromPauseMenu)
{
	OptionCount = KEY_COUNT + 1;

	CurrentSettings.WaitingForKey = false;

	if (CurrentSettings.IgnoreInput)
	{
		if (NoInput())
			CurrentSettings.IgnoreInput = false;

		return;
	}

	if (GUI_INPUT_SELECT && SelectedOption <= 16)
	{
		SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
		CurrentSettings.WaitingForKey = true;
		CurrentSettings.IgnoreInput = true;
	}

	if (CurrentSettings.WaitingForKey)
	{
		ClearInput();

		while (true)
		{
			UpdateInput();

			if (CurrentSettings.IgnoreInput)
			{
				if (NoInput())
					CurrentSettings.IgnoreInput = false;
			}
			else
			{
				int selectedKey = 0;
				for (selectedKey = 0; selectedKey < MAX_INPUT_SLOTS; selectedKey++)
				{
					if (KeyMap[selectedKey])
						break;
				}

				if (selectedKey == MAX_INPUT_SLOTS)
					selectedKey = 0;

				if (selectedKey && g_KeyNames[selectedKey])
				{
					KeyboardLayout[1][SelectedOption] = selectedKey;
					DefaultConflict();

					CurrentSettings.WaitingForKey = false;
					CurrentSettings.IgnoreInput = true;
					return;
				}
			}

			if (fromPauseMenu)
			{
				g_Renderer.RenderInventory();
				Camera.numberFrames = g_Renderer.SyncRenderer();
			}
			else
			{
				g_Renderer.RenderTitle();
				Camera.numberFrames = g_Renderer.SyncRenderer();
				int numFrames = Camera.numberFrames;
				ControlPhase(numFrames, 0);
			}
		}
	}
	else
	{
		if (GUI_INPUT_PULSE_UP)
		{
			if (SelectedOption <= 0)
				SelectedOption += OptionCount;
			else
				SelectedOption--;

			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}

		if (GUI_INPUT_PULSE_DOWN)
		{
			if (SelectedOption < OptionCount)
				SelectedOption++;
			else
				SelectedOption -= OptionCount;

			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}

		if (GUI_INPUT_SELECT)
		{
			// Apply
			if (SelectedOption == OptionCount - 1)
			{
				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
				memcpy(CurrentSettings.Configuration.KeyboardLayout, KeyboardLayout[1], KEY_COUNT * sizeof(short));
				memcpy(g_Configuration.KeyboardLayout, KeyboardLayout[1], KEY_COUNT * sizeof(short));
				SaveConfiguration();
				MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
				SelectedOption = 2;
				return;
			}

			// Cancel
			if (SelectedOption == OptionCount)
			{
				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
				memcpy(KeyboardLayout[1], CurrentSettings.Configuration.KeyboardLayout, KEY_COUNT * sizeof(short));
				MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
				SelectedOption = 2;
				return;
			}
		}

		if (GUI_INPUT_DESELECT)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

			MenuToDisplay = Menu::Options;
			SelectedOption = 2;
		}
	}
}

void GuiController::BackupOptions()
{
	memcpy(&CurrentSettings.Configuration, &g_Configuration, sizeof(GameConfiguration));
}

void GuiController::HandleOptionsInput()
{
	switch (SelectedOption)
	{
	case 0:
		FillDisplayOptions();
		MenuToDisplay = Menu::Display;
		SelectedOption = 0;
		break;

	case 1:
		BackupOptions();
		MenuToDisplay = Menu::OtherSettings;
		SelectedOption = 0;
		break;

	case 2:
		BackupOptions();
		MenuToDisplay = Menu::Controls;
		SelectedOption = 0;
		break;
	}
}

void GuiController::HandleOtherSettingsInput(bool fromPauseMenu)
{
	OptionCount = 7;
	if (GUI_INPUT_DESELECT)
	{
		SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

		MenuToDisplay = Menu::Options;
		SelectedOption = 1;

		SetVolumeMusic(g_Configuration.MusicVolume);
		SetVolumeFX(g_Configuration.SfxVolume);
		return;
	}

	if (GUI_INPUT_PULSE_LEFT || GUI_INPUT_PULSE_RIGHT)
	{
		switch (SelectedOption)
		{

		case 0:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.EnableReverb = !CurrentSettings.Configuration.EnableReverb;
			break;

		case 3:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.AutoTarget = !CurrentSettings.Configuration.AutoTarget;
			break;

		case 4:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.EnableRumble = !CurrentSettings.Configuration.EnableRumble;
			break;

		case 5:
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			CurrentSettings.Configuration.EnableThumbstickCameraControl = !CurrentSettings.Configuration.EnableThumbstickCameraControl;
			break;
		}
	}

	if (GUI_INPUT_PULSE_LEFT)
	{
		switch (SelectedOption)
		{
		case 1:
			if (CurrentSettings.Configuration.MusicVolume > 0)
			{
				if (IsHeld(In::Action))
					CurrentSettings.Configuration.MusicVolume -= 3;
				else
					CurrentSettings.Configuration.MusicVolume--;

				if (CurrentSettings.Configuration.MusicVolume < 0)
					CurrentSettings.Configuration.MusicVolume = 0;

				SetVolumeMusic(CurrentSettings.Configuration.MusicVolume);
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}

			break;

		case 2:
			if (CurrentSettings.Configuration.SfxVolume > 0)
			{
				if (IsHeld(In::Action))
					CurrentSettings.Configuration.SfxVolume -= 3;
				else
					CurrentSettings.Configuration.SfxVolume--;

				if (CurrentSettings.Configuration.SfxVolume < 0)
					CurrentSettings.Configuration.SfxVolume = 0;

				SetVolumeFX(CurrentSettings.Configuration.SfxVolume);
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}

			break;
		}
	}

	if (GUI_INPUT_PULSE_RIGHT)
	{
		switch (SelectedOption)
		{
		case 1:
			if (CurrentSettings.Configuration.MusicVolume < VOLUME_MAX)
			{
				if (IsHeld(In::Action))
					CurrentSettings.Configuration.MusicVolume += 3;
				else
					CurrentSettings.Configuration.MusicVolume++;

				if (CurrentSettings.Configuration.MusicVolume > VOLUME_MAX)
					CurrentSettings.Configuration.MusicVolume = VOLUME_MAX;

				SetVolumeMusic(CurrentSettings.Configuration.MusicVolume);
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}

			break;

		case 2:
			if (CurrentSettings.Configuration.SfxVolume < VOLUME_MAX)
			{
				if (IsHeld(In::Action))
					CurrentSettings.Configuration.SfxVolume += 3;
				else
					CurrentSettings.Configuration.SfxVolume++;

				if (CurrentSettings.Configuration.SfxVolume > VOLUME_MAX)
					CurrentSettings.Configuration.SfxVolume = VOLUME_MAX;

				SetVolumeFX(CurrentSettings.Configuration.SfxVolume);
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}

			break;
		}
	}

	if (GUI_INPUT_PULSE_UP)
	{
		if (SelectedOption <= 0)
			SelectedOption += OptionCount;
		else
			SelectedOption--;

		SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
	}

	if (GUI_INPUT_PULSE_DOWN)
	{
		if (SelectedOption < OptionCount)
			SelectedOption++;
		else
			SelectedOption -= OptionCount;

		SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
	}

	if (GUI_INPUT_SELECT)
	{
		SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

		if (SelectedOption == OptionCount - 1)
		{
			// Was rumble setting changed?
			bool indicateRumble = CurrentSettings.Configuration.EnableRumble && !g_Configuration.EnableRumble;

			// Save the configuration
			memcpy(&g_Configuration, &CurrentSettings.Configuration, sizeof(GameConfiguration));
			SaveConfiguration();

			// Rumble if setting was changed
			if (indicateRumble)
				Rumble(0.5f);

			MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
			SelectedOption = 1;
		}
		else if (SelectedOption == OptionCount)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			SetVolumeMusic(g_Configuration.MusicVolume);
			SetVolumeFX(g_Configuration.SfxVolume);
			MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
			SelectedOption = 1;
		}
	}
}

InventoryResult GuiController::DoPauseMenu()
{
	UpdateInput();

	switch (MenuToDisplay)
	{
	case Menu::Pause:
		OptionCount = 2;
		break;

	case Menu::Statistics:
		OptionCount = 0;
		break;

	case Menu::Options:
		OptionCount = 2;
		break;

	case Menu::Display:
		HandleDisplaySettingsInput(true);
		break;

	case Menu::Controls:
		HandleControlSettingsInput(true);
		break;

	case Menu::OtherSettings:
		HandleOtherSettingsInput(true);
		break;
	}

	if (MenuToDisplay == Menu::Pause || MenuToDisplay == Menu::Options)
	{
		if (GUI_INPUT_PULSE_UP)
		{
			if (SelectedOption <= 0)
				SelectedOption += OptionCount;
			else
				SelectedOption--;

			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}

		if (GUI_INPUT_PULSE_DOWN)
		{
			if (SelectedOption < OptionCount)
				SelectedOption++;
			else
				SelectedOption -= OptionCount;

			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}
	}

	if (GUI_INPUT_DESELECT || IsClicked(In::Pause))
	{
		if (MenuToDisplay == Menu::Pause)
		{
			InvMode = InventoryMode::None;
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			return InventoryResult::None;
		}

		if (MenuToDisplay == Menu::Statistics || MenuToDisplay == Menu::Options)
		{
			SelectedOption = MenuToDisplay == Menu::Statistics ? 0 : 1;
			MenuToDisplay = Menu::Pause;
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
		}
	}

	if (GUI_INPUT_SELECT)
	{
		switch (MenuToDisplay)
		{
		case Menu::Pause:

			switch (SelectedOption)
			{
			case 0:
				SelectedOption = 0;
				MenuToDisplay = Menu::Statistics;
				break;

			case 1:
				SelectedOption = 0;
				MenuToDisplay = Menu::Options;
				break;

			case 2:
				InvMode = InventoryMode::None;
				return InventoryResult::ExitToTitle;
			}

			break;

		case Menu::Options:
			HandleOptionsInput();
			break;

		case Menu::Statistics:
			MenuToDisplay = Menu::Pause;
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			break;
		}
	}

	return InventoryResult::None;
}

//----------
// Inventory
//----------

bool GuiController::DoObjectsCombine(int objectNumber1, int objectNumber2)
{
	for (int n = 0; n < MAX_COMBINES; n++)
	{
		if (CombineTable[n].item1 == objectNumber1 &&
			CombineTable[n].item2 == objectNumber2)
			return true;

		if (CombineTable[n].item1 == objectNumber2 &&
			CombineTable[n].item2 == objectNumber1)
			return true;
	}

	return false;
}

bool GuiController::IsItemCurrentlyCombinable(int objectNumber)
{
	if (objectNumber < INV_OBJECT_SMALL_WATERSKIN || objectNumber > INV_OBJECT_BIG_WATERSKIN5L)//trash
	{
		for (int n = 0; n < MAX_COMBINES; n++)
		{
			if (CombineTable[n].item1 == objectNumber)
			{
				if (IsItemInInventory(CombineTable[n].item2))
					return true;
			}

			if (CombineTable[n].item2 == objectNumber)
			{
				if (IsItemInInventory(CombineTable[n].item1))
					return true;
			}
		}
	}
	else if (objectNumber > INV_OBJECT_SMALL_WATERSKIN3L)
	{
		for (int n = 0; n < 4; n++)
		{
			if (IsItemInInventory(n + INV_OBJECT_SMALL_WATERSKIN))
				return true;
		}
	}
	else
	{
		for (int n = 0; n < 6; n++)
		{
			if (IsItemInInventory(n + INV_OBJECT_BIG_WATERSKIN))
				return true;
		}
	}

	return false;
}

bool GuiController::IsItemInInventory(int objectNumber)
{
	for (int i = 0; i < INVENTORY_TABLE_SIZE; i++)
	{
		if (Rings[(int)RingTypes::Inventory]->CurrentObjectList[i].invitem == objectNumber)
			return 1;
	}

	return 0;
}

void GuiController::CombineObjects(int objectNumber1, int objectNumber2)
{
	int n;
	for (n = 0; n < MAX_COMBINES; n++)
	{
		if (CombineTable[n].item1 == objectNumber1 &&
			CombineTable[n].item2 == objectNumber2)
			break;

		if (CombineTable[n].item1 == objectNumber2 &&
			CombineTable[n].item2 == objectNumber1)
			break;
	}

	CombineTable[n].CombineRoutine(0);
	ConstructObjectList();
	SetupObjectListStartPosition(CombineTable[n].combined_item);
	HandleObjectChangeover((int)RingTypes::Inventory);
}

void GuiController::SeparateObject(int objectNumber)
{
	int n;
	for (n = 0; n < MAX_COMBINES; n++)
	{
		if (CombineTable[n].combined_item == objectNumber)
			break;
	}

	CombineTable[n].CombineRoutine(1);
	ConstructObjectList();
	SetupObjectListStartPosition(CombineTable[n].item1);
}

void GuiController::SetupObjectListStartPosition(int newObjectNumber)
{
	for (int i = 0; i < INVENTORY_TABLE_SIZE; i++)
	{
		if (Rings[(int)RingTypes::Inventory]->CurrentObjectList[i].invitem == newObjectNumber)
			Rings[(int)RingTypes::Inventory]->curobjinlist = i;
	}
}

void GuiController::HandleObjectChangeover(int ringIndex)
{
	CurrentSelectedOption = 0;
	MenuActive = 1;
	SetupAmmoSelector();
}

void GuiController::SetupAmmoSelector()
{
	int number;
	unsigned __int64 options;

	number = 0;
	options = InventoryObjectTable[Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->curobjinlist].invitem].opts;
	AmmoSelectorFlag = 0;
	NumAmmoSlots = 0;

	if (Rings[(int)RingTypes::Ammo]->ringactive)
		return;
	
	AmmoObjectList[0].xrot = 0;
	AmmoObjectList[0].yrot = 0;
	AmmoObjectList[0].zrot = 0;
	AmmoObjectList[1].xrot = 0;
	AmmoObjectList[1].yrot = 0;
	AmmoObjectList[1].zrot = 0;
	AmmoObjectList[2].xrot = 0;
	AmmoObjectList[2].yrot = 0;
	AmmoObjectList[2].zrot = 0;

	if (options & 
		(OPT_CHOOSEAMMO_UZI | OPT_CHOOSEAMMO_PISTOLS | OPT_CHOOSEAMMO_REVOLVER | OPT_CHOOSEAMMO_CROSSBOW |
		OPT_CHOOSEAMMO_HK | OPT_CHOOSEAMMO_SHOTGUN | OPT_CHOOSEAMMO_GRENADEGUN | OPT_CHOOSEAMMO_HARPOON | OPT_CHOOSEAMMO_ROCKET))
	{
		AmmoSelectorFlag = 1;
		AmmoSelectorFadeDir = 1;

		if (options & OPT_CHOOSEAMMO_UZI)
		{
			AmmoObjectList[0].invitem = INV_OBJECT_UZI_AMMO;
			AmmoObjectList[0].amount = Ammo.AmountUziAmmo;
			number++;
			NumAmmoSlots = number;
			CurrentAmmoType = &Ammo.CurrentUziAmmoType;
		}

		if (options & OPT_CHOOSEAMMO_PISTOLS)
		{
			number++;
			AmmoObjectList[0].invitem = INV_OBJECT_PISTOLS_AMMO;
			AmmoObjectList[0].amount = -1;
			NumAmmoSlots = number;
			CurrentAmmoType = &Ammo.CurrentPistolsAmmoType;
		}

		if (options & OPT_CHOOSEAMMO_REVOLVER)
		{
			number++;
			AmmoObjectList[0].invitem = INV_OBJECT_REVOLVER_AMMO;
			AmmoObjectList[0].amount = Ammo.AmountRevolverAmmo;
			NumAmmoSlots = number;
			CurrentAmmoType = &Ammo.CurrentRevolverAmmoType;
		}

		if (options & OPT_CHOOSEAMMO_CROSSBOW)
		{
			AmmoObjectList[number].invitem = INV_OBJECT_CROSSBOW_AMMO1;
			AmmoObjectList[number].amount = Ammo.AmountCrossBowAmmo1;
			number++;
			AmmoObjectList[number].invitem = INV_OBJECT_CROSSBOW_AMMO2;
			AmmoObjectList[number].amount = Ammo.AmountCrossBowAmmo2;
			number++;
			AmmoObjectList[number].invitem = INV_OBJECT_CROSSBOW_AMMO3;
			AmmoObjectList[number].amount = Ammo.AmountCrossBowAmmo3;
			number++;
			NumAmmoSlots = number;
			CurrentAmmoType = &Ammo.CurrentCrossBowAmmoType;
		}

		if (options & OPT_CHOOSEAMMO_HK)
		{
			AmmoObjectList[number].invitem = INV_OBJECT_HK_AMMO;
			AmmoObjectList[number].amount = Ammo.AmountHKAmmo1;
			number++;
			NumAmmoSlots = number;
			CurrentAmmoType = &Ammo.CurrentHKAmmoType;
		}

		if (options & OPT_CHOOSEAMMO_SHOTGUN)
		{
			AmmoObjectList[number].invitem = INV_OBJECT_SHOTGUN_AMMO1;
			AmmoObjectList[number].amount = Ammo.AmountShotGunAmmo1;
			number++;
			AmmoObjectList[number].invitem = INV_OBJECT_SHOTGUN_AMMO2;
			AmmoObjectList[number].amount = Ammo.AmountShotGunAmmo2;
			number++;
			NumAmmoSlots = number;
			CurrentAmmoType = &Ammo.CurrentShotGunAmmoType;
		}

		if (options & OPT_CHOOSEAMMO_GRENADEGUN)
		{
			AmmoObjectList[number].invitem = INV_OBJECT_GRENADE_AMMO1;
			AmmoObjectList[number].amount = Ammo.AmountGrenadeAmmo1;
			number++;
			AmmoObjectList[number].invitem = INV_OBJECT_GRENADE_AMMO2;
			AmmoObjectList[number].amount = Ammo.AmountGrenadeAmmo2;
			number++;
			AmmoObjectList[number].invitem = INV_OBJECT_GRENADE_AMMO3;
			AmmoObjectList[number].amount = Ammo.AmountGrenadeAmmo3;
			number++;
			NumAmmoSlots = number;
			CurrentAmmoType = &Ammo.CurrentGrenadeGunAmmoType;
		}

		if (options & OPT_CHOOSEAMMO_HARPOON)
		{
			AmmoObjectList[number].invitem = INV_OBJECT_HARPOON_AMMO;
			AmmoObjectList[number].amount = Ammo.AmountHarpoonAmmo;
			number++;
			NumAmmoSlots = number;
			CurrentAmmoType = &Ammo.CurrentHarpoonAmmoType;
		}

		if (options & OPT_CHOOSEAMMO_ROCKET)
		{
			AmmoObjectList[number].invitem = INV_OBJECT_ROCKET_AMMO;
			AmmoObjectList[number].amount = Ammo.AmountRocketsAmmo;
			number++;
			NumAmmoSlots = number;
			CurrentAmmoType = &Ammo.CurrentRocketAmmoType;
		}
	}
}

void GuiController::InsertObjectIntoList(int objectNumber)
{
	Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->numobjectsinlist].invitem = objectNumber;
	Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->numobjectsinlist].xrot = 0;
	Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->numobjectsinlist].yrot = 0;
	Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->numobjectsinlist].zrot = 0;
	Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->numobjectsinlist].bright = 32;
	Rings[(int)RingTypes::Inventory]->numobjectsinlist++;
}

void GuiController::InsertObjectIntoList_v2(int objectNumber)
{
	unsigned __int64 opts = InventoryObjectTable[objectNumber].opts;

	if (opts & (OPT_COMBINABLE | OPT_ALWAYSCOMBINE))
	{
		if (Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->curobjinlist].invitem != objectNumber)
		{
			Rings[(int)RingTypes::Ammo]->CurrentObjectList[Rings[(int)RingTypes::Ammo]->numobjectsinlist].invitem = objectNumber;
			Rings[(int)RingTypes::Ammo]->CurrentObjectList[Rings[(int)RingTypes::Ammo]->numobjectsinlist].xrot = 0;
			Rings[(int)RingTypes::Ammo]->CurrentObjectList[Rings[(int)RingTypes::Ammo]->numobjectsinlist].yrot = 0;
			Rings[(int)RingTypes::Ammo]->CurrentObjectList[Rings[(int)RingTypes::Ammo]->numobjectsinlist].zrot = 0;
			Rings[(int)RingTypes::Ammo]->CurrentObjectList[Rings[(int)RingTypes::Ammo]->numobjectsinlist++].bright = 32;
		}
	}
}

void GuiController::ConstructObjectList()
{
	Rings[(int)RingTypes::Inventory]->numobjectsinlist = 0;

	for (int i = 0; i < INVENTORY_TABLE_SIZE; i++)
		Rings[(int)RingTypes::Inventory]->CurrentObjectList[i].invitem = NO_ITEM;

	Ammo.CurrentPistolsAmmoType = 0;
	Ammo.CurrentUziAmmoType = 0;
	Ammo.CurrentRevolverAmmoType = 0;
	Ammo.CurrentShotGunAmmoType = 0;
	Ammo.CurrentGrenadeGunAmmoType = 0;
	Ammo.CurrentCrossBowAmmoType = 0;

	if (!(g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young))
	{
		if (Lara.Weapons[(int)LaraWeaponType::Pistol].Present)
			InsertObjectIntoList(INV_OBJECT_PISTOLS);
		else if (Ammo.AmountPistolsAmmo)
			InsertObjectIntoList(INV_OBJECT_PISTOLS_AMMO);

		if (Lara.Weapons[(int)LaraWeaponType::Uzi].Present)
			InsertObjectIntoList(INV_OBJECT_UZIS);
		else if (Ammo.AmountUziAmmo)
			InsertObjectIntoList(INV_OBJECT_UZI_AMMO);

		if (Lara.Weapons[(int)LaraWeaponType::Revolver].Present)
		{
			if (Lara.Weapons[(int)LaraWeaponType::Revolver].HasLasersight)
				InsertObjectIntoList(INV_OBJECT_REVOLVER_LASER);
			else
				InsertObjectIntoList(INV_OBJECT_REVOLVER);
		}
		else if (Ammo.AmountRevolverAmmo)
			InsertObjectIntoList(INV_OBJECT_REVOLVER_AMMO);

		if (Lara.Weapons[(int)LaraWeaponType::Shotgun].Present)
		{
			InsertObjectIntoList(INV_OBJECT_SHOTGUN);

			if (Lara.Weapons[(int)LaraWeaponType::Shotgun].SelectedAmmo == WeaponAmmoType::Ammo2)
				Ammo.CurrentShotGunAmmoType = 1;
		}
		else
		{
			if (Ammo.AmountShotGunAmmo1)
				InsertObjectIntoList(INV_OBJECT_SHOTGUN_AMMO1);

			if (Ammo.AmountShotGunAmmo2)
				InsertObjectIntoList(INV_OBJECT_SHOTGUN_AMMO2);
		}

		if (Lara.Weapons[(int)LaraWeaponType::HK].Present)
		{
			if (Lara.Weapons[(int)LaraWeaponType::HK].HasSilencer)
				InsertObjectIntoList(INV_OBJECT_HK_SILENCER);
			else
				InsertObjectIntoList(INV_OBJECT_HK);
		}
		else if (Ammo.AmountHKAmmo1)
			InsertObjectIntoList(INV_OBJECT_HK_AMMO);

		if (Lara.Weapons[(int)LaraWeaponType::Crossbow].Present)
		{
				if (Lara.Weapons[(int)LaraWeaponType::Crossbow].HasLasersight)
					InsertObjectIntoList(INV_OBJECT_CROSSBOW_LASER);
				else
					InsertObjectIntoList(INV_OBJECT_CROSSBOW);

				if (Lara.Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo == WeaponAmmoType::Ammo2)
					Ammo.CurrentCrossBowAmmoType = 1;

				if (Lara.Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo == WeaponAmmoType::Ammo3)
					Ammo.CurrentCrossBowAmmoType = 2;
		}
		else
		{
			if (Ammo.AmountCrossBowAmmo1)
				InsertObjectIntoList(INV_OBJECT_CROSSBOW_AMMO1);

			if (Ammo.AmountCrossBowAmmo2)
				InsertObjectIntoList(INV_OBJECT_CROSSBOW_AMMO2);

			if (Ammo.AmountCrossBowAmmo3)
				InsertObjectIntoList(INV_OBJECT_CROSSBOW_AMMO3);
		}

		if (Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Present)
		{
			InsertObjectIntoList(INV_OBJECT_GRENADE_LAUNCHER);

			if (Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo == WeaponAmmoType::Ammo2)
				Ammo.CurrentGrenadeGunAmmoType = 1;

			if (Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo == WeaponAmmoType::Ammo3)
				Ammo.CurrentGrenadeGunAmmoType = 2;
		}
		else
		{
			if (Ammo.AmountGrenadeAmmo1)
				InsertObjectIntoList(INV_OBJECT_GRENADE_AMMO1);

			if (Ammo.AmountGrenadeAmmo2)
				InsertObjectIntoList(INV_OBJECT_GRENADE_AMMO2);

			if (Ammo.AmountGrenadeAmmo3)
				InsertObjectIntoList(INV_OBJECT_GRENADE_AMMO3);
		}

		if (Lara.Weapons[(int)LaraWeaponType::RocketLauncher].Present)
			InsertObjectIntoList(INV_OBJECT_ROCKET_LAUNCHER);
		else if (Ammo.AmountRocketsAmmo)
			InsertObjectIntoList(INV_OBJECT_ROCKET_AMMO);

		if (Lara.Weapons[(int)LaraWeaponType::HarpoonGun].Present)
			InsertObjectIntoList(INV_OBJECT_HARPOON_GUN);
		else if (Ammo.AmountHarpoonAmmo)
			InsertObjectIntoList(INV_OBJECT_HARPOON_AMMO);

		if (Lara.Inventory.HasLasersight)
			InsertObjectIntoList(INV_OBJECT_LASERSIGHT);

		if (Lara.Inventory.HasSilencer)
			InsertObjectIntoList(INV_OBJECT_SILENCER);

		if (Lara.Inventory.HasBinoculars)
			InsertObjectIntoList(INV_OBJECT_BINOCULARS);

		if (Lara.Inventory.TotalFlares)
			InsertObjectIntoList(INV_OBJECT_FLARES);
	}

	InsertObjectIntoList(INV_OBJECT_TIMEX);//every level has the timex? what's a good way to check?!

	if (Lara.Inventory.TotalSmallMedipacks)
		InsertObjectIntoList(INV_OBJECT_SMALL_MEDIPACK);

	if (Lara.Inventory.TotalLargeMedipacks)
		InsertObjectIntoList(INV_OBJECT_LARGE_MEDIPACK);

	if (Lara.Inventory.HasCrowbar)
		InsertObjectIntoList(INV_OBJECT_CROWBAR);

	if (Lara.Inventory.BeetleComponents)
	{
		if (Lara.Inventory.BeetleComponents & 1)
			InsertObjectIntoList(INV_OBJECT_BEETLE);

		if (Lara.Inventory.BeetleComponents & 2)
			InsertObjectIntoList(INV_OBJECT_BEETLE_PART1);

		if (Lara.Inventory.BeetleComponents & 4)
			InsertObjectIntoList(INV_OBJECT_BEETLE_PART2);
	}

	if (Lara.Inventory.SmallWaterskin)
		InsertObjectIntoList((Lara.Inventory.SmallWaterskin - 1) + INV_OBJECT_SMALL_WATERSKIN);

	if (Lara.Inventory.BigWaterskin)
		InsertObjectIntoList((Lara.Inventory.BigWaterskin - 1) + INV_OBJECT_BIG_WATERSKIN);

	for (int i = 0; i < NUM_PUZZLES; i++)
	{
		if (Lara.Inventory.Puzzles[i])
			InsertObjectIntoList(INV_OBJECT_PUZZLE1 + i);
	}

	for (int i = 0; i < NUM_PUZZLE_PIECES; i++)
	{
		if (Lara.Inventory.PuzzlesCombo[i])
			InsertObjectIntoList(INV_OBJECT_PUZZLE1_COMBO1 + i);
	}

	for (int i = 0; i < NUM_KEYS; i++)
	{
		if (Lara.Inventory.Keys[i])
			InsertObjectIntoList(INV_OBJECT_KEY1 + i);
	}

	for (int i = 0; i < NUM_KEY_PIECES; i++)
	{
		if (Lara.Inventory.KeysCombo[i])
			InsertObjectIntoList(INV_OBJECT_KEY1_COMBO1 + i);
	}

	for (int i = 0; i < NUM_PICKUPS; i++)
	{
		if (Lara.Inventory.Pickups[i])
			InsertObjectIntoList(INV_OBJECT_PICKUP1 + i);
	}

	for (int i = 0; i < NUM_PICKUPS_PIECES; i++)
	{
		if (Lara.Inventory.PickupsCombo[i])
			InsertObjectIntoList(INV_OBJECT_PICKUP1_COMBO1 + i);
	}

	for (int i = 0; i < NUM_EXAMINES; i++)
	{
		if (Lara.Inventory.Examines[i])
			InsertObjectIntoList(INV_OBJECT_EXAMINE1 + i);
	}

	for (int i = 0; i < NUM_EXAMINES_PIECES; i++)
	{
		if (Lara.Inventory.ExaminesCombo[i])
			InsertObjectIntoList(INV_OBJECT_EXAMINE1_COMBO1 + i);
	}

	if (Lara.Inventory.Diary.Present)
		InsertObjectIntoList(INV_OBJECT_DIARY);

	if (g_GameFlow->EnableLoadSave)
	{
		InsertObjectIntoList(INV_OBJECT_LOAD_FLOPPY);
		InsertObjectIntoList(INV_OBJECT_SAVE_FLOPPY);
	}

	Rings[(int)RingTypes::Inventory]->objlistmovement = 0;
	Rings[(int)RingTypes::Inventory]->curobjinlist = 0;
	Rings[(int)RingTypes::Inventory]->ringactive = 1;
	Rings[(int)RingTypes::Ammo]->objlistmovement = 0;
	Rings[(int)RingTypes::Ammo]->curobjinlist = 0;
	Rings[(int)RingTypes::Ammo]->ringactive = 0;
	HandleObjectChangeover((int)RingTypes::Inventory);
	AmmoActive = 0;
}

void GuiController::ConstructCombineObjectList()
{
	Rings[(int)RingTypes::Ammo]->numobjectsinlist = 0;

	for (int i = 0; i < INVENTORY_TABLE_SIZE; i++)
		Rings[(int)RingTypes::Ammo]->CurrentObjectList[i].invitem = NO_ITEM;

	if (!(g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young))
	{
		if (Lara.Weapons[(int)LaraWeaponType::Revolver].Present)
		{
			if (Lara.Weapons[(int)LaraWeaponType::Revolver].HasLasersight)
				InsertObjectIntoList_v2(INV_OBJECT_REVOLVER_LASER);
			else
				InsertObjectIntoList_v2(INV_OBJECT_REVOLVER);
		}

		if (Lara.Weapons[(int)LaraWeaponType::HK].Present)
			InsertObjectIntoList_v2(INV_OBJECT_HK);

		if (Lara.Weapons[(int)LaraWeaponType::Crossbow].Present)
		{
			if (Lara.Weapons[(int)LaraWeaponType::Crossbow].HasLasersight)
				InsertObjectIntoList_v2(INV_OBJECT_CROSSBOW_LASER);
			else
				InsertObjectIntoList_v2(INV_OBJECT_CROSSBOW);
		}

		if (Lara.Inventory.HasLasersight)
			InsertObjectIntoList_v2(INV_OBJECT_LASERSIGHT);

		if (Lara.Inventory.HasSilencer)
			InsertObjectIntoList_v2(INV_OBJECT_SILENCER);
	}

	if (Lara.Inventory.BeetleComponents)
	{
		if (Lara.Inventory.BeetleComponents & 2)
			InsertObjectIntoList_v2(INV_OBJECT_BEETLE_PART1);

		if (Lara.Inventory.BeetleComponents & 4)
			InsertObjectIntoList_v2(INV_OBJECT_BEETLE_PART2);
	}

	if (Lara.Inventory.SmallWaterskin)
		InsertObjectIntoList_v2(Lara.Inventory.SmallWaterskin - 1 + INV_OBJECT_SMALL_WATERSKIN);

	if (Lara.Inventory.BigWaterskin)
		InsertObjectIntoList_v2(Lara.Inventory.BigWaterskin - 1 + INV_OBJECT_BIG_WATERSKIN);

	for (int i = 0; i < NUM_PUZZLE_PIECES; i++)
	{
		if (Lara.Inventory.PuzzlesCombo[i])
			InsertObjectIntoList_v2(INV_OBJECT_PUZZLE1_COMBO1 + i);
	}

	for (int i = 0; i < NUM_KEY_PIECES; i++)
	{
		if (Lara.Inventory.KeysCombo[i])
			InsertObjectIntoList_v2(INV_OBJECT_KEY1_COMBO1 + i);
	}

	for (int i = 0; i < NUM_PICKUPS_PIECES; i++)
	{
		if (Lara.Inventory.PickupsCombo[i])
			InsertObjectIntoList_v2(INV_OBJECT_PICKUP1_COMBO1 + i);
	}

	for (int i = 0; i < NUM_EXAMINES_PIECES; i++)
	{
		if (Lara.Inventory.ExaminesCombo[i])
			InsertObjectIntoList_v2(INV_OBJECT_EXAMINE1_COMBO1 + i);
	}

	Rings[(int)RingTypes::Ammo]->objlistmovement = 0;
	Rings[(int)RingTypes::Ammo]->curobjinlist = 0;
	Rings[(int)RingTypes::Ammo]->ringactive = 0;
}

void GuiController::InitialiseInventory()
{
	CompassNeedleAngle = ANGLE(22.5f);
	AlterFOV(ANGLE(80.0f));
	Lara.Inventory.IsBusy = false;
	InventoryItemChosen = NO_ITEM;
	UseItem = 0;

	if (Lara.Weapons[(int)LaraWeaponType::Shotgun].Ammo[0].hasInfinite())
		Ammo.AmountShotGunAmmo1 = -1;
	else
		Ammo.AmountShotGunAmmo1 = Lara.Weapons[(int)LaraWeaponType::Shotgun].Ammo[0].getCount() / 6;

	if (Lara.Weapons[(int)LaraWeaponType::Shotgun].Ammo[1].hasInfinite())
		Ammo.AmountShotGunAmmo2 = -1;
	else
		Ammo.AmountShotGunAmmo2 = Lara.Weapons[(int)LaraWeaponType::Shotgun].Ammo[1].getCount() / 6;

	Ammo.AmountHKAmmo1 = Lara.Weapons[(int)LaraWeaponType::HK].Ammo[(int)WeaponAmmoType::Ammo1].hasInfinite() ? -1 : Lara.Weapons[(int)LaraWeaponType::HK].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
	Ammo.AmountCrossBowAmmo1 = Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo1].hasInfinite() ? -1 : Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
	Ammo.AmountCrossBowAmmo2 = Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo2].hasInfinite() ? -1 : Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo2].getCount();
	Ammo.AmountCrossBowAmmo3 = Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo3].hasInfinite() ? -1 : Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo3].getCount();
	Ammo.AmountUziAmmo = Lara.Weapons[(int)LaraWeaponType::Uzi].Ammo[(int)WeaponAmmoType::Ammo1].hasInfinite() ? -1 : Lara.Weapons[(int)LaraWeaponType::Uzi].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
	Ammo.AmountRevolverAmmo = Lara.Weapons[(int)LaraWeaponType::Revolver].Ammo[(int)WeaponAmmoType::Ammo1].hasInfinite() ? -1 : Lara.Weapons[(int)LaraWeaponType::Revolver].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
	Ammo.AmountPistolsAmmo = Lara.Weapons[(int)LaraWeaponType::Pistol].Ammo[(int)WeaponAmmoType::Ammo1].hasInfinite() ? -1 : Lara.Weapons[(int)LaraWeaponType::Pistol].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
	Ammo.AmountRocketsAmmo = Lara.Weapons[(int)LaraWeaponType::RocketLauncher].Ammo[(int)WeaponAmmoType::Ammo1].hasInfinite() ? -1 : Lara.Weapons[(int)LaraWeaponType::RocketLauncher].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
	Ammo.AmountHarpoonAmmo = Lara.Weapons[(int)LaraWeaponType::HarpoonGun].Ammo[(int)WeaponAmmoType::Ammo1].hasInfinite()? -1 : Lara.Weapons[(int)LaraWeaponType::HarpoonGun].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
	Ammo.AmountGrenadeAmmo1 = Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo1].hasInfinite()? -1 : Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
	Ammo.AmountGrenadeAmmo2 = Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo2].hasInfinite() ? -1 : Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo2].getCount();
	Ammo.AmountGrenadeAmmo3 = Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo3].hasInfinite() ? -1 : Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo3].getCount();
	ConstructObjectList();

	if (EnterInventory == NO_ITEM)
	{
		if (LastInvItem != NO_ITEM)
		{
			if (IsItemInInventory(LastInvItem))
				SetupObjectListStartPosition(LastInvItem);
			else
			{
				if (LastInvItem >= INV_OBJECT_SMALL_WATERSKIN && LastInvItem <= INV_OBJECT_SMALL_WATERSKIN3L)
				{
					for (int i = INV_OBJECT_SMALL_WATERSKIN; i <= INV_OBJECT_SMALL_WATERSKIN3L; i++)
					{
						if (IsItemInInventory(i))
						{
							SetupObjectListStartPosition(i);
							break;
						}
					}
				}
				else if (LastInvItem >= INV_OBJECT_BIG_WATERSKIN && LastInvItem <= INV_OBJECT_BIG_WATERSKIN5L)
				{
					for (int i = INV_OBJECT_BIG_WATERSKIN; i <= INV_OBJECT_BIG_WATERSKIN5L; i++)
					{
						if (IsItemInInventory(i))
						{
							SetupObjectListStartPosition(i);
							break;
						}
					}
				}
			}

			LastInvItem = NO_ITEM;
		}
	}
	else
	{
		if (IsObjectInInventory(EnterInventory))
			SetupObjectListStartPosition2(EnterInventory);

		EnterInventory = NO_ITEM;
	}

	AmmoSelectorFadeVal = 0;
	AmmoSelectorFadeDir = 0;
	CombineRingFadeVal = 0;
	CombineRingFadeDir = 0;
	CombineTypeFlag = 0;
	SeperateTypeFlag = 0;
	CombineObject1 = 0;
	CombineObject2 = 0;
	NormalRingFadeVal = 128;
	NormalRingFadeDir = 0;
	HandleObjectChangeover((int)RingTypes::Inventory);
}

int GuiController::IsObjectInInventory(int objectNumber)
{
	return GetInventoryCount(from_underlying((short)objectNumber));
}

void GuiController::SetupObjectListStartPosition2(int newObjectNumber)
{
	for (int i = 0; i < INVENTORY_TABLE_SIZE; i++)
	{
		if (InventoryObjectTable[Rings[(int)RingTypes::Inventory]->CurrentObjectList[i].invitem].object_number == newObjectNumber)
			Rings[(int)RingTypes::Inventory]->curobjinlist = i;
	}
}

int GuiController::ConvertObjectToInventoryItem(int objectNumber)
{
	for (int i = 0; i < INVENTORY_TABLE_SIZE; i++)
	{
		if (InventoryObjectTable[i].object_number == objectNumber)
			return i;
	}

	return -1;
}

int GuiController::ConvertInventoryItemToObject(int objectNumber)
{
	return InventoryObjectTable[objectNumber].object_number;
}

void GuiController::FadeAmmoSelector()
{
	if (Rings[(int)RingTypes::Inventory]->ringactive)
		AmmoSelectorFadeVal = 0;
	else if (AmmoSelectorFadeDir == 1)
	{
		if (AmmoSelectorFadeVal < 128)
			AmmoSelectorFadeVal += 32;

		if (AmmoSelectorFadeVal > 128)
		{
			AmmoSelectorFadeVal = 128;
			AmmoSelectorFadeDir = 0;
		}
	}
	else if (AmmoSelectorFadeDir == 2)
	{
		if (AmmoSelectorFadeVal > 0)
			AmmoSelectorFadeVal -= 32;

		if (AmmoSelectorFadeVal < 0)
		{
			AmmoSelectorFadeVal = 0;
			AmmoSelectorFadeDir = 0;
		}
	}
}

void GuiController::UseCurrentItem()
{
	short invobject, gmeobject;
	long OldBinocular;

	OldBinocular = BinocularRange;
	Lara.Inventory.OldBusy = false;
	BinocularRange = 0;
	LaraItem->MeshBits = ALL_JOINT_BITS;
	invobject = Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->curobjinlist].invitem;
	gmeobject = InventoryObjectTable[invobject].object_number;

	if (Lara.Control.WaterStatus == WaterStatus::Dry || Lara.Control.WaterStatus == WaterStatus::Wade)
	{
		if (gmeobject == ID_PISTOLS_ITEM)
		{
			Lara.Control.Weapon.RequestGunType = LaraWeaponType::Pistol;

			if (Lara.Control.HandStatus != HandStatus::Free)
				return;

			if (Lara.Control.Weapon.GunType == LaraWeaponType::Pistol)
				Lara.Control.HandStatus = HandStatus::WeaponDraw;

			return;
		}

		if (gmeobject == ID_UZI_ITEM)
		{
			Lara.Control.Weapon.RequestGunType = LaraWeaponType::Uzi;

			if (Lara.Control.HandStatus != HandStatus::Free)
				return;

			if (Lara.Control.Weapon.GunType == LaraWeaponType::Uzi)
				Lara.Control.HandStatus = HandStatus::WeaponDraw;

			return;
		}
	}

	if (gmeobject != ID_SHOTGUN_ITEM && gmeobject != ID_REVOLVER_ITEM && gmeobject != ID_HK_ITEM && gmeobject != ID_CROSSBOW_ITEM &&
		gmeobject != ID_GRENADE_GUN_ITEM && gmeobject != ID_ROCKET_LAUNCHER_ITEM && gmeobject != ID_HARPOON_ITEM)
	{
		if (gmeobject == ID_FLARE_INV_ITEM)
		{
			if (Lara.Control.HandStatus == HandStatus::Free)
			{
				if (LaraItem->Animation.ActiveState != LS_CRAWL_IDLE &&
					LaraItem->Animation.ActiveState != LS_CRAWL_FORWARD &&
					LaraItem->Animation.ActiveState != LS_CRAWL_TURN_LEFT &&
					LaraItem->Animation.ActiveState != LS_CRAWL_TURN_RIGHT &&
					LaraItem->Animation.ActiveState != LS_CRAWL_BACK &&
					LaraItem->Animation.ActiveState != LS_CRAWL_TO_HANG)
				{
					if (Lara.Control.Weapon.GunType != LaraWeaponType::Flare)
					{
						// HACK.
						ClearInput();
						ActionMap[(int)In::Flare].Update(1.0f);
						TrInput = IN_FLARE;

						LaraGun(LaraItem);
						ClearInput();
					}

					return;
				}
			}

			SayNo();
			return;
		}

		switch (invobject)
		{
		case INV_OBJECT_BINOCULARS:
			if (((LaraItem->Animation.ActiveState == LS_IDLE && LaraItem->Animation.AnimNumber == LA_STAND_IDLE) ||
					(Lara.Control.IsLow && !IsHeld(In::Crouch))) &&
				!UseSpotCam && !TrackCameraInit)
			{
				BinocularRange = 128;
				BinocularOn = true;
				Lara.Inventory.OldBusy = true;

				// TODO: To prevent Lara from crouching or performing other actions, the inherent state of
				// LA_BINOCULARS_IDLE must be changed to LS_IDLE. @Sezz 2022.05.19
				//SetAnimation(LaraItem, LA_BINOCULARS_IDLE);

				if (Lara.Control.HandStatus != HandStatus::Free)
					Lara.Control.HandStatus = HandStatus::WeaponUndraw;
			}

			if (OldBinocular)
				BinocularRange = OldBinocular;
			else
				BinocularOldCamera = Camera.oldType;

			return;

		case INV_OBJECT_SMALL_MEDIPACK:

			if ((LaraItem->HitPoints <= 0 || LaraItem->HitPoints >= LARA_HEALTH_MAX) && !Lara.PoisonPotency)
			{
				SayNo();
				return;
			}

			if (Lara.Inventory.TotalSmallMedipacks != 0)
			{
				if (Lara.Inventory.TotalSmallMedipacks != -1)
					Lara.Inventory.TotalSmallMedipacks--;

				Lara.PoisonPotency = 0;
				LaraItem->HitPoints += LARA_HEALTH_MAX / 2;

				if (LaraItem->HitPoints > LARA_HEALTH_MAX)
					LaraItem->HitPoints = LARA_HEALTH_MAX;

				SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always);
				Statistics.Game.HealthUsed++;
			}
			else
				SayNo();

			return;

		case INV_OBJECT_LARGE_MEDIPACK:

			if ((LaraItem->HitPoints <= 0 || LaraItem->HitPoints >= LARA_HEALTH_MAX) && !Lara.PoisonPotency)
			{
				SayNo();
				return;
			}

			if (Lara.Inventory.TotalLargeMedipacks != 0)
			{
				if (Lara.Inventory.TotalLargeMedipacks != -1)
					Lara.Inventory.TotalLargeMedipacks--;

				Lara.PoisonPotency = 0;
				LaraItem->HitPoints = LARA_HEALTH_MAX;

				SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always);
				Statistics.Game.HealthUsed++;
			}
			else
				SayNo();

			return;

		default:
			InventoryItemChosen = gmeobject;
			return;
		}

		return;
	}

	if (Lara.Control.HandStatus == HandStatus::Busy)
	{
		SayNo();
		return;
	}

	if (LaraItem->Animation.ActiveState == LS_CRAWL_IDLE ||
		LaraItem->Animation.ActiveState == LS_CRAWL_FORWARD ||
		LaraItem->Animation.ActiveState == LS_CRAWL_TURN_LEFT ||
		LaraItem->Animation.ActiveState == LS_CRAWL_TURN_RIGHT ||
		LaraItem->Animation.ActiveState == LS_CRAWL_BACK ||
		LaraItem->Animation.ActiveState == LS_CRAWL_TO_HANG ||
		LaraItem->Animation.ActiveState == LS_CROUCH_IDLE ||
		LaraItem->Animation.ActiveState == LS_CROUCH_TURN_LEFT ||
		LaraItem->Animation.ActiveState == LS_CROUCH_TURN_RIGHT)
	{
		SayNo();
		return;
	}

	if (gmeobject == ID_SHOTGUN_ITEM)
	{
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::Shotgun;

		if (Lara.Control.HandStatus != HandStatus::Free)
			return;

		if (Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun)
			Lara.Control.HandStatus = HandStatus::WeaponDraw;

		return;
	}

	if (gmeobject == ID_REVOLVER_ITEM)
	{
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::Revolver;

		if (Lara.Control.HandStatus != HandStatus::Free)
			return;

		if (Lara.Control.Weapon.GunType == LaraWeaponType::Revolver)
			Lara.Control.HandStatus = HandStatus::WeaponDraw;

		return;
	}
	else if (gmeobject == ID_HK_ITEM)
	{
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::HK;

		if (Lara.Control.HandStatus != HandStatus::Free)
			return;

		if (Lara.Control.Weapon.GunType == LaraWeaponType::HK)
			Lara.Control.HandStatus = HandStatus::WeaponDraw;

		return;
	}
	else if (gmeobject == ID_CROSSBOW_ITEM)
	{
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::Crossbow;

		if (Lara.Control.HandStatus != HandStatus::Free)
			return;

		if (Lara.Control.Weapon.GunType == LaraWeaponType::Crossbow)
			Lara.Control.HandStatus = HandStatus::WeaponDraw;

		return;
	}
	else if (gmeobject == ID_GRENADE_GUN_ITEM)
	{
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::GrenadeLauncher;

		if (Lara.Control.HandStatus != HandStatus::Free)
			return;

		if (Lara.Control.Weapon.GunType == LaraWeaponType::GrenadeLauncher)
			Lara.Control.HandStatus = HandStatus::WeaponDraw;

		return;
	}
	else if (gmeobject == ID_HARPOON_ITEM)
	{
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::HarpoonGun;

		if (Lara.Control.HandStatus != HandStatus::Free)
			return;

		if (Lara.Control.Weapon.GunType == LaraWeaponType::HarpoonGun)
			Lara.Control.HandStatus = HandStatus::WeaponDraw;

		return;
	}
	else if (gmeobject == ID_ROCKET_LAUNCHER_ITEM)
	{
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::RocketLauncher;

		if (Lara.Control.HandStatus != HandStatus::Free)
			return;

		if (Lara.Control.Weapon.GunType == LaraWeaponType::RocketLauncher)
			Lara.Control.HandStatus = HandStatus::WeaponDraw;

		return;
	}
}

void GuiController::DoInventory()
{
	if (Rings[(int)RingTypes::Ammo]->ringactive)
	{
		g_Renderer.DrawString(PHD_CENTER_X, PHD_CENTER_Y, g_GameFlow->GetString(OptionStrings[5]), PRINTSTRING_COLOR_WHITE, PRINTSTRING_BLINK | PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);

		if (Rings[(int)RingTypes::Inventory]->objlistmovement)
			return;

		if (Rings[(int)RingTypes::Ammo]->objlistmovement)
			return;

		if (GUI_INPUT_SELECT)
		{
			short invItem = Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->curobjinlist].invitem;
			short ammoItem = Rings[(int)RingTypes::Ammo]->CurrentObjectList[Rings[(int)RingTypes::Ammo]->curobjinlist].invitem;

			if (DoObjectsCombine(invItem, ammoItem))
			{
				CombineRingFadeDir = 2;
				CombineTypeFlag = 1;
				CombineObject1 = invItem;
				CombineObject2 = ammoItem;
				SoundEffect(SFX_TR4_MENU_COMBINE, nullptr, SoundEnvironment::Always);
			}
			else if (ammoItem >= INV_OBJECT_SMALL_WATERSKIN &&
				ammoItem <= INV_OBJECT_SMALL_WATERSKIN3L &&
				invItem >= INV_OBJECT_BIG_WATERSKIN &&
				invItem <= INV_OBJECT_BIG_WATERSKIN5L)
			{
				if (PerformWaterskinCombine(1))
				{
					CombineTypeFlag = 2;
					CombineRingFadeDir = 2;
					SoundEffect(SFX_TR4_MENU_COMBINE, nullptr, SoundEnvironment::Always);
					return;
				}

				SayNo();
				CombineRingFadeDir = 2;
			}
			else if (invItem >= INV_OBJECT_SMALL_WATERSKIN &&
				invItem <= INV_OBJECT_SMALL_WATERSKIN3L &&
				ammoItem >= INV_OBJECT_BIG_WATERSKIN &&
				ammoItem <= INV_OBJECT_BIG_WATERSKIN5L)
			{
				if (PerformWaterskinCombine(0))
				{
					CombineTypeFlag = 2;
					CombineRingFadeDir = 2;
					SoundEffect(SFX_TR4_MENU_COMBINE, nullptr, SoundEnvironment::Always);
					return;
				}

				SayNo();
				CombineRingFadeDir = 2;
			}
			else
			{
				SayNo();
				CombineRingFadeDir = 2;
			}
		}

		if (GUI_INPUT_DESELECT)
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			CombineRingFadeDir = 2;
		}

		return;
	}
	else
	{
		int num = Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->curobjinlist].invitem;

		for (int i = 0; i < 3; i++)
		{
			CurrentOptions[i].type = MenuType::None;
			CurrentOptions[i].text = 0;
		}

		int n = 0;
		unsigned long options;
		if (!AmmoActive)
		{
			options = InventoryObjectTable[Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->curobjinlist].invitem].opts;

			if (options & OPT_LOAD)
			{
				CurrentOptions[0].type = MenuType::Load;
				CurrentOptions[0].text = g_GameFlow->GetString(OptionStrings[6]);
				n = 1;
			}

			if (options & OPT_SAVE)
			{
				CurrentOptions[n].type = MenuType::Save;
				CurrentOptions[n].text = g_GameFlow->GetString(OptionStrings[7]);
				n++;
			}

			if (options & OPT_EXAMINABLE)
			{
				CurrentOptions[n].type = MenuType::Examine;
				CurrentOptions[n].text = g_GameFlow->GetString(OptionStrings[8]);
				n++;
			}

			if (options & OPT_STATS)
			{
				CurrentOptions[n].type = MenuType::Statistics;
				CurrentOptions[n].text = g_GameFlow->GetString(OptionStrings[9]);
				n++;
			}

			if (options & OPT_USE)
			{
				CurrentOptions[n].type = MenuType::Use;
				CurrentOptions[n].text = g_GameFlow->GetString(OptionStrings[0]);
				n++;
			}

			if (options & OPT_EQUIP)
			{
				CurrentOptions[n].type = MenuType::Equip;
				CurrentOptions[n].text = g_GameFlow->GetString(OptionStrings[4]);
				n++;
			}

			if (options & (OPT_CHOOSEAMMO_SHOTGUN | OPT_CHOOSEAMMO_CROSSBOW | OPT_CHOOSEAMMO_GRENADEGUN))
			{
				CurrentOptions[n].type = MenuType::ChooseAmmo;
				CurrentOptions[n].text = g_GameFlow->GetString(OptionStrings[1]);
				n++;
			}

			if (options & OPT_COMBINABLE)
			{
				if (IsItemCurrentlyCombinable(num))
				{
					CurrentOptions[n].type = MenuType::Combine;
					CurrentOptions[n].text = g_GameFlow->GetString(OptionStrings[2]);
					n++;
				}
			}

			if (options & OPT_ALWAYSCOMBINE)
			{
				CurrentOptions[n].type = MenuType::Combine;
				CurrentOptions[n].text = g_GameFlow->GetString(OptionStrings[2]);
				n++;
			}

			if (options & OPT_SEPERATABLE)
			{
				CurrentOptions[n].type = MenuType::Seperate;
				CurrentOptions[n].text = g_GameFlow->GetString(OptionStrings[3]);
				n++;
			}

			if (options & OPT_DIARY)
			{
				CurrentOptions[n].type = MenuType::Diary;
				CurrentOptions[n].text = g_GameFlow->GetString(OptionStrings[11]);
				n++;
			}
		}
		else
		{
			CurrentOptions[0].type = MenuType::Ammo1;
			CurrentOptions[0].text = g_GameFlow->GetString(InventoryObjectTable[AmmoObjectList[0].invitem].objname);
			CurrentOptions[1].type = MenuType::Ammo2;
			CurrentOptions[1].text = g_GameFlow->GetString(InventoryObjectTable[AmmoObjectList[1].invitem].objname);
			n = 2;

			options = InventoryObjectTable[Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->curobjinlist].invitem].opts;

			if (options & (OPT_CHOOSEAMMO_CROSSBOW | OPT_CHOOSEAMMO_GRENADEGUN))
			{
				n = 3;
				CurrentOptions[2].type = MenuType::Ammo3;
				CurrentOptions[2].text = g_GameFlow->GetString(InventoryObjectTable[AmmoObjectList[2].invitem].objname);
			}

			CurrentSelectedOption = *CurrentAmmoType;
		}

		int yPos = 310 - LINE_HEIGHT;

		if (n == 1)
			yPos += LINE_HEIGHT;
		else if (n == 2)
			yPos += LINE_HEIGHT >> 1;

		if (n > 0)
		{
			for (int i = 0; i < n; i++)
			{
				if (i == CurrentSelectedOption)
				{
					g_Renderer.DrawString(PHD_CENTER_X, yPos, CurrentOptions[i].text, PRINTSTRING_COLOR_WHITE, PRINTSTRING_BLINK | PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
					yPos += LINE_HEIGHT;
				}
				else
				{
					g_Renderer.DrawString(PHD_CENTER_X, yPos, CurrentOptions[i].text, PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
					yPos += LINE_HEIGHT;
				}
			}
		}

		if (MenuActive &&
			!Rings[(int)RingTypes::Inventory]->objlistmovement &&
			!Rings[(int)RingTypes::Ammo]->objlistmovement)
		{
			if (GUI_INPUT_PULSE_UP)
			{
				if (CurrentSelectedOption <= 0)
					CurrentSelectedOption = n - 1;
				else
					CurrentSelectedOption--;

				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			}
			else if (GUI_INPUT_PULSE_DOWN)
			{
				if (CurrentSelectedOption >= n - 1)
					CurrentSelectedOption = 0;
				else
					CurrentSelectedOption++;

				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			}

			if (AmmoActive)
			{
				if (GUI_INPUT_PULSE_LEFT)
				{
					if (CurrentSelectedOption <= 0)
						CurrentSelectedOption = n - 1;
					else
						CurrentSelectedOption--;

					SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
				}

				if (GUI_INPUT_PULSE_RIGHT)
				{
					if (CurrentSelectedOption >= n - 1)
						CurrentSelectedOption = 0;
					else
						CurrentSelectedOption++;

					SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
				}

				*CurrentAmmoType = CurrentSelectedOption;
			}

			if (GUI_INPUT_SELECT)
			{
				if (CurrentOptions[CurrentSelectedOption].type != MenuType::Equip && CurrentOptions[CurrentSelectedOption].type != MenuType::Use)
					SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

				switch (CurrentOptions[CurrentSelectedOption].type)
				{
				case MenuType::ChooseAmmo:
					Rings[(int)RingTypes::Inventory]->ringactive = 0;
					AmmoActive = 1;
					Ammo.StashedCurrentSelectedOption = CurrentSelectedOption;
					Ammo.StashedCurrentPistolsAmmoType = Ammo.CurrentPistolsAmmoType;
					Ammo.StashedCurrentUziAmmoType = Ammo.CurrentUziAmmoType;
					Ammo.StashedCurrentRevolverAmmoType = Ammo.CurrentRevolverAmmoType;
					Ammo.StashedCurrentShotGunAmmoType = Ammo.CurrentShotGunAmmoType;
					Ammo.StashedCurrentGrenadeGunAmmoType = Ammo.CurrentGrenadeGunAmmoType;
					Ammo.StashedCurrentCrossBowAmmoType = Ammo.CurrentCrossBowAmmoType;
					Ammo.StashedCurrentHKAmmoType = Ammo.CurrentHKAmmoType;
					Ammo.StashedCurrentHarpoonAmmoType = Ammo.CurrentHarpoonAmmoType;
					Ammo.StashedCurrentRocketAmmoType = Ammo.CurrentRocketAmmoType;
					break;

				case MenuType::Load:
					//fill_up_savegames_array//or maybe not?
					InvMode = InventoryMode::Load;
					break;

				case MenuType::Save:
					//fill_up_savegames_array
					InvMode = InventoryMode::Save;
					break;

				case MenuType::Examine:
					InvMode = InventoryMode::Examine;
					break;

				case MenuType::Statistics:
					InvMode = InventoryMode::Statistics;
					break;

				case MenuType::Ammo1:
				case MenuType::Ammo2:
				case MenuType::Ammo3:
					AmmoActive = 0;
					Rings[(int)RingTypes::Inventory]->ringactive = 1;
					CurrentSelectedOption = 0;
					break;

				case MenuType::Combine:
					ConstructCombineObjectList();
					Rings[(int)RingTypes::Inventory]->ringactive = 0;
					Rings[(int)RingTypes::Ammo]->ringactive = 1;
					AmmoSelectorFlag = 0;
					MenuActive = 0;
					CombineRingFadeDir = 1;
					break;

				case MenuType::Seperate:
					SeperateTypeFlag = 1;
					NormalRingFadeDir = 2;
					break;

				case MenuType::Equip:
				case MenuType::Use:
					MenuActive = 0;
					UseItem = 1;
					break;

				case MenuType::Diary:
					InvMode = InventoryMode::Diary;
					Lara.Inventory.Diary.currentPage = 1;
					break;
				}
			}

			if (GUI_INPUT_DESELECT && AmmoActive)
			{
				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
				AmmoActive = 0;
				Rings[(int)RingTypes::Inventory]->ringactive = 1;
				Ammo.CurrentPistolsAmmoType = Ammo.StashedCurrentPistolsAmmoType;
				Ammo.CurrentUziAmmoType = Ammo.StashedCurrentUziAmmoType;
				Ammo.CurrentRevolverAmmoType = Ammo.StashedCurrentRevolverAmmoType;
				Ammo.CurrentShotGunAmmoType = Ammo.StashedCurrentShotGunAmmoType;
				Ammo.CurrentGrenadeGunAmmoType = Ammo.StashedCurrentGrenadeGunAmmoType;
				Ammo.CurrentCrossBowAmmoType = Ammo.StashedCurrentCrossBowAmmoType;
				Ammo.CurrentHKAmmoType = Ammo.StashedCurrentHKAmmoType;
				Ammo.CurrentHarpoonAmmoType = Ammo.StashedCurrentHarpoonAmmoType;
				Ammo.CurrentRocketAmmoType = Ammo.StashedCurrentRocketAmmoType;
				CurrentSelectedOption = Ammo.StashedCurrentSelectedOption;
			}
		}
	}
}

// Update the selected ammo of weapons that require it, and only these.
void GuiController::UpdateWeaponStatus()
{
	if (Lara.Weapons[(int)LaraWeaponType::Shotgun].Present)
	{
		if (Ammo.CurrentShotGunAmmoType)
			Lara.Weapons[(int)LaraWeaponType::Shotgun].SelectedAmmo = WeaponAmmoType::Ammo2;
		else
			Lara.Weapons[(int)LaraWeaponType::Shotgun].SelectedAmmo = WeaponAmmoType::Ammo1;
	}

	if (Lara.Weapons[(int)LaraWeaponType::Crossbow].Present)
	{
		Lara.Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo = WeaponAmmoType::Ammo1;

		if (Ammo.CurrentCrossBowAmmoType == 1)
			Lara.Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo = WeaponAmmoType::Ammo2;
		else if (Ammo.CurrentCrossBowAmmoType == 2)
			Lara.Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo = WeaponAmmoType::Ammo3;
	}

	if (Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Present)
	{
		Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo = WeaponAmmoType::Ammo1;

		if (Ammo.CurrentGrenadeGunAmmoType == 1)
			Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo = WeaponAmmoType::Ammo2;
		else if (Ammo.CurrentGrenadeGunAmmoType == 2)
			Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo = WeaponAmmoType::Ammo3;
	}
}

void GuiController::SpinBack(unsigned short* angle)
{
	unsigned short val;
	unsigned short val2;

	val = *angle;

	if (val)
	{
		if (val <= 32768)
		{
			val2 = val;

			if (val2 < ANGLE(5.0f))
				val = ANGLE(5.0f);
			else if (val2 > ANGLE(90.0f))
				val2 = ANGLE(90.0f);

			val -= (val2 >> 3);

			if (val > 32768)
				val = 0;
		}
		else
		{
			val2 = -val;

			if (val2 < ANGLE(5.0f))
				val = ANGLE(5.0f);
			else if (val2 > ANGLE(90.0f))
				val2 = ANGLE(90.0f);

			val += (val2 >> 3);

			if (val < ANGLE(180.0f))
				val = 0;
		}

		*angle = val;
	}
}

void GuiController::DrawAmmoSelector()
{
	int n;
	unsigned short xrot, yrot, zrot;
	InventoryObject* objme;
	char invTextBuffer[256];
	int x, y;

	if (!AmmoSelectorFlag)
		return;
	
	int xPos = (2 * PHD_CENTER_X - OBJLIST_SPACING) >> 1;

	if (NumAmmoSlots == 2)
		xPos -= OBJLIST_SPACING / 2;
	else if (NumAmmoSlots == 3)
		xPos -= OBJLIST_SPACING;

	if (NumAmmoSlots > 0)
	{
		for (n = 0; n < NumAmmoSlots; n++)
		{
			objme = &InventoryObjectTable[AmmoObjectList[n].invitem];

			if (n == *CurrentAmmoType)
			{
				if (objme->rot_flags & INV_ROT_X)
					AmmoObjectList[n].xrot += ANGLE(5.0f);

				if (objme->rot_flags & INV_ROT_Y)
					AmmoObjectList[n].yrot += ANGLE(5.0f);

				if (objme->rot_flags & INV_ROT_Z)
					AmmoObjectList[n].zrot += ANGLE(5.0f);
			}
			else
			{
				SpinBack(&AmmoObjectList[n].xrot);
				SpinBack(&AmmoObjectList[n].yrot);
				SpinBack(&AmmoObjectList[n].zrot);
			}

			xrot = AmmoObjectList[n].xrot;
			yrot = AmmoObjectList[n].yrot;
			zrot = AmmoObjectList[n].zrot;
			x = PHD_CENTER_X - 300 + xPos;
			y = 480;
			short obj = ConvertInventoryItemToObject(AmmoObjectList[n].invitem);
			float scaler = InventoryObjectTable[AmmoObjectList[n].invitem].scale1;

			if (n == *CurrentAmmoType)
			{
				if (AmmoObjectList[n].amount == -1)
					sprintf(&invTextBuffer[0], "Unlimited %s", g_GameFlow->GetString(InventoryObjectTable[AmmoObjectList[n].invitem].objname));
				else
					sprintf(&invTextBuffer[0], "%d x %s", AmmoObjectList[n].amount, g_GameFlow->GetString(InventoryObjectTable[AmmoObjectList[n].invitem].objname));

				if (AmmoSelectorFadeVal)
					g_Renderer.DrawString(PHD_CENTER_X, 380, &invTextBuffer[0], PRINTSTRING_COLOR_YELLOW, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);

				
				if (n == *CurrentAmmoType)
					g_Renderer.DrawObjectOn2DPosition(x, y, obj, xrot, yrot, zrot, scaler);
				else
					g_Renderer.DrawObjectOn2DPosition(x, y, obj, xrot, yrot, zrot, scaler);
			}
			else
				g_Renderer.DrawObjectOn2DPosition(x, y, obj, xrot, yrot, zrot, scaler);

			xPos += OBJLIST_SPACING;
		}
	}
}

void GuiController::DrawCurrentObjectList(int ringIndex)
{
	char textbufme[128];
	unsigned short xrot, yrot, zrot;

	if (Rings[ringIndex]->CurrentObjectList <= 0)
		return;

	if (ringIndex == (int)RingTypes::Ammo)
	{
		AmmoSelectorFadeVal = 0;
		AmmoSelectorFadeDir = 0;

		if (CombineRingFadeDir == 1)
		{
			if (CombineRingFadeVal < 128)
				CombineRingFadeVal += 32;

			if (CombineRingFadeVal > 128)
			{
				CombineRingFadeVal = 128;
				CombineRingFadeDir = 0;
			}
		}
		else if (CombineRingFadeDir == 2)
		{
			CombineRingFadeVal -= 32;

			if (CombineRingFadeVal <= 0)
			{
				CombineRingFadeVal = 0;
				CombineRingFadeDir = 0;

				if (CombineTypeFlag)
					NormalRingFadeDir = 2;
				else
				{
					Rings[(int)RingTypes::Inventory]->ringactive = 1;
					MenuActive = 1;
					Rings[(int)RingTypes::Ammo]->ringactive = 0;
					HandleObjectChangeover((int)RingTypes::Inventory);
				}

				Rings[(int)RingTypes::Ammo]->ringactive = 0;
			}
		}
	}
	else if (NormalRingFadeDir == 1)
	{
		if (NormalRingFadeVal < 128)
			NormalRingFadeVal += 32;

		if (NormalRingFadeVal > 128)
		{
			NormalRingFadeVal = 128;
			NormalRingFadeDir = 0;
			Rings[(int)RingTypes::Inventory]->ringactive = 1;
			MenuActive = 1;
		}

	}
	else if (NormalRingFadeDir == 2)
	{
		NormalRingFadeVal -= 32;

		if (NormalRingFadeVal <= 0)
		{
			NormalRingFadeVal = 0;
			NormalRingFadeDir = 1;

			if (CombineTypeFlag == 1)
			{
				CombineTypeFlag = 0;
				CombineObjects(CombineObject1, CombineObject2);
			}
			else if (CombineTypeFlag == 2)
			{
				CombineTypeFlag = 0;
				ConstructObjectList();
				SetupObjectListStartPosition(CombineObject1);
			}
			else if (SeperateTypeFlag)
				SeparateObject(Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->curobjinlist].invitem);

			HandleObjectChangeover((int)RingTypes::Inventory);
		}
	}

	int minObj = 0;
	int maxObj = 0;
	int xOffset = 0;
	int n = 0;

	if (Rings[ringIndex]->numobjectsinlist != 1)
		xOffset = (OBJLIST_SPACING * Rings[ringIndex]->objlistmovement) >> 16;

	if (Rings[ringIndex]->numobjectsinlist == 2)
	{
		minObj = -1;
		maxObj = 0;
		n = Rings[ringIndex]->curobjinlist - 1;
	}

	if (Rings[ringIndex]->numobjectsinlist == 3 || Rings[ringIndex]->numobjectsinlist == 4)
	{
		minObj = -2;
		maxObj = 1;
		n = Rings[ringIndex]->curobjinlist - 2;
	}

	if (Rings[ringIndex]->numobjectsinlist >= 5)
	{
		minObj = -3;
		maxObj = 2;
		n = Rings[ringIndex]->curobjinlist - 3;
	}

	if (n < 0)
		n += Rings[ringIndex]->numobjectsinlist;

	if (Rings[ringIndex]->objlistmovement < 0)
		maxObj++;

	if (minObj <= maxObj)
	{
		for (int i = minObj; i <= maxObj; i++)
		{
			int shade = 0;

			if (minObj == i)
			{
				if (Rings[ringIndex]->objlistmovement < 0)
					shade = 0;
				else
					shade = Rings[ringIndex]->objlistmovement >> 9;
			}
			else if (i != minObj + 1 || maxObj == minObj + 1)
			{
				if (i != maxObj)
					shade = 128;
				else
				{
					if (Rings[ringIndex]->objlistmovement < 0)
						shade = (-128 * Rings[ringIndex]->objlistmovement) >> 16;
					else
						shade = 128 - (short)(Rings[ringIndex]->objlistmovement >> 9);
				}
			}
			else
			{
				if (Rings[ringIndex]->objlistmovement < 0)
					shade = 128 - ((-128 * Rings[ringIndex]->objlistmovement) >> 16);
				else
					shade = 128;
			}

			if (!minObj && !maxObj)
				shade = 128;

			if (ringIndex == (int)RingTypes::Ammo && CombineRingFadeVal < 128 && shade)
				shade = CombineRingFadeVal;
			else if (ringIndex == (int)RingTypes::Inventory && NormalRingFadeVal < 128 && shade)
				shade = NormalRingFadeVal;

			if (!i)
			{
				int numMeUp = 0;
				int count = 0;

				switch (InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].object_number)
				{
				case ID_BIGMEDI_ITEM:
					numMeUp = Lara.Inventory.TotalLargeMedipacks;
					break;

				case ID_SMALLMEDI_ITEM:
					numMeUp = Lara.Inventory.TotalSmallMedipacks;
					break;

				case ID_FLARE_INV_ITEM:
					numMeUp = Lara.Inventory.TotalFlares;
					break;

				default:
					if (InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].object_number < ID_PUZZLE_ITEM1 ||
						InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].object_number > ID_PUZZLE_ITEM16)
					{
						switch (InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].object_number)
						{
						case ID_SHOTGUN_AMMO1_ITEM:
							count = Lara.Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
							numMeUp = count == -1 ? count : count / 6;
							break;

						case ID_SHOTGUN_AMMO2_ITEM:
							count = Lara.Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo2].getCount();
							numMeUp = count == -1 ? count : count / 6;
							break;

						case ID_HK_AMMO_ITEM:
							numMeUp = Lara.Weapons[(int)LaraWeaponType::HK].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
							break;

						case ID_CROSSBOW_AMMO1_ITEM:
							count = Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
							numMeUp = count;
							break;

						case ID_CROSSBOW_AMMO2_ITEM:
							count = Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo2].getCount();
							numMeUp = count;
							break;

						case ID_CROSSBOW_AMMO3_ITEM:
							count = Lara.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo3].getCount();
							numMeUp = count;
							break;

						case ID_GRENADE_AMMO1_ITEM:
							count = Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
							numMeUp = count;
							break;

						case ID_GRENADE_AMMO2_ITEM:
							count = Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo2].getCount();
							numMeUp = count;
							break;

						case ID_GRENADE_AMMO3_ITEM:
							count = Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo3].getCount();
							numMeUp = count;
							break;

						case ID_ROCKET_LAUNCHER_ITEM:
							numMeUp = Lara.Weapons[(int)LaraWeaponType::RocketLauncher].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
							break;

						case ID_HARPOON_ITEM:
							numMeUp = Lara.Weapons[(int)LaraWeaponType::HarpoonGun].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
							break;

						case ID_REVOLVER_AMMO_ITEM:
							numMeUp = Lara.Weapons[(int)LaraWeaponType::Revolver].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
							break;

						case ID_UZI_AMMO_ITEM:
							numMeUp = Lara.Weapons[(int)LaraWeaponType::Uzi].Ammo[(int)WeaponAmmoType::Ammo1].getCount();
							break;
						}
					}
					else
					{
						numMeUp = Lara.Inventory.Puzzles[InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].object_number - ID_PUZZLE_ITEM1];

						if (numMeUp <= 1)
							sprintf(textbufme, g_GameFlow->GetString(InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].objname));
						else
							sprintf(textbufme, "%d x %s", numMeUp, g_GameFlow->GetString(InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].objname));
					}

					break;
				}

				if (InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].object_number < ID_PUZZLE_ITEM1 ||
					InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].object_number > ID_PUZZLE_ITEM16)
				{
					if (numMeUp)
					{
						if (numMeUp == -1)
							sprintf(textbufme, "Unlimited %s", g_GameFlow->GetString(InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].objname));
						else
							sprintf(textbufme, "%d x %s", numMeUp, g_GameFlow->GetString(InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].objname));
					}
					else
						sprintf(textbufme, g_GameFlow->GetString(InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].objname));
				}

				int objmeup;
				if (ringIndex == (int)RingTypes::Inventory)
					objmeup = (int)(PHD_CENTER_Y - (REFERENCE_RES_HEIGHT + 1) * 0.0625 * 2.5);
				else
					objmeup = (int)(PHD_CENTER_Y + (REFERENCE_RES_HEIGHT + 1) * 0.0625 * 2.0);

				g_Renderer.DrawString(PHD_CENTER_X, objmeup, textbufme, PRINTSTRING_COLOR_YELLOW, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
			}

			if (!i && !Rings[ringIndex]->objlistmovement)
			{
				if (InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].rot_flags & INV_ROT_X)
					Rings[ringIndex]->CurrentObjectList[n].xrot += ANGLE(5.0f);

				if (InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].rot_flags & INV_ROT_Y)
					Rings[ringIndex]->CurrentObjectList[n].yrot += ANGLE(5.0f);

				if (InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].rot_flags & INV_ROT_Z)
					Rings[ringIndex]->CurrentObjectList[n].zrot += ANGLE(5.0f);
			}
			else
			{
				SpinBack(&Rings[ringIndex]->CurrentObjectList[n].xrot);
				SpinBack(&Rings[ringIndex]->CurrentObjectList[n].yrot);
				SpinBack(&Rings[ringIndex]->CurrentObjectList[n].zrot);
			}

			xrot = Rings[ringIndex]->CurrentObjectList[n].xrot;
			yrot = Rings[ringIndex]->CurrentObjectList[n].yrot;
			zrot = Rings[ringIndex]->CurrentObjectList[n].zrot;

			int activeNum = 0;
			if (Rings[ringIndex]->objlistmovement)
			{
				if (Rings[ringIndex]->objlistmovement > 0)
					activeNum = -1;
				else
					activeNum = 1;
			}

			if (i == activeNum)
			{
				if (Rings[ringIndex]->CurrentObjectList[n].bright < 160)
					Rings[ringIndex]->CurrentObjectList[n].bright += 16;

				if (Rings[ringIndex]->CurrentObjectList[n].bright > 160)
					Rings[ringIndex]->CurrentObjectList[n].bright = 160;
			}
			else
			{
				if (Rings[ringIndex]->CurrentObjectList[n].bright > 32)
					Rings[ringIndex]->CurrentObjectList[n].bright -= 16;

				if (Rings[ringIndex]->CurrentObjectList[n].bright < 32)
					Rings[ringIndex]->CurrentObjectList[n].bright = 32;
			}

			int x, y, y2;
			x = 400 + xOffset + i * OBJLIST_SPACING;
			y = 150;
			y2 = 480;//combine 
			short obj = ConvertInventoryItemToObject(Rings[ringIndex]->CurrentObjectList[n].invitem);
			float scaler = InventoryObjectTable[Rings[ringIndex]->CurrentObjectList[n].invitem].scale1;
			g_Renderer.DrawObjectOn2DPosition(x, ringIndex == (int)RingTypes::Inventory ? y : y2, obj, xrot, yrot, zrot, scaler);

			if (++n >= Rings[ringIndex]->numobjectsinlist)
				n = 0;
		}

		if (Rings[ringIndex]->ringactive)
		{
			if (Rings[ringIndex]->numobjectsinlist != 1 && (ringIndex != 1 || CombineRingFadeVal == 128))
			{
				if (Rings[ringIndex]->objlistmovement > 0)
					Rings[ringIndex]->objlistmovement += 8192;

				if (Rings[ringIndex]->objlistmovement < 0)
					Rings[ringIndex]->objlistmovement -= 8192;

				if (IsHeld(In::Left))
				{
					if (!Rings[ringIndex]->objlistmovement)
					{
						SoundEffect(SFX_TR4_MENU_ROTATE, nullptr, SoundEnvironment::Always);
						Rings[ringIndex]->objlistmovement += 8192;

						if (AmmoSelectorFlag)
							AmmoSelectorFadeDir = 2;
					}
				}

				if (IsHeld(In::Right))
				{
					if (!Rings[ringIndex]->objlistmovement)
					{
						SoundEffect(SFX_TR4_MENU_ROTATE, nullptr, SoundEnvironment::Always);
						Rings[ringIndex]->objlistmovement -= 8192;

						if (AmmoSelectorFlag)
							AmmoSelectorFadeDir = 2;
					}

				}

				if (Rings[ringIndex]->objlistmovement < 65536)
				{
					if (Rings[ringIndex]->objlistmovement < -65535)
					{
						Rings[ringIndex]->curobjinlist++;

						if (Rings[ringIndex]->curobjinlist >= Rings[ringIndex]->numobjectsinlist)
							Rings[ringIndex]->curobjinlist = 0;

						Rings[ringIndex]->objlistmovement = 0;

						if (ringIndex == (int)RingTypes::Inventory)
							HandleObjectChangeover(0);
					}
				}
				else
				{
					Rings[ringIndex]->curobjinlist--;

					if (Rings[ringIndex]->curobjinlist < 0)
						Rings[ringIndex]->curobjinlist = Rings[ringIndex]->numobjectsinlist - 1;

					Rings[ringIndex]->objlistmovement = 0;

					if (ringIndex == (int)RingTypes::Inventory)
						HandleObjectChangeover(0);
				}
			}
		}
	}
}

bool GuiController::CallInventory(bool resetMode)
{
	bool returnValue = false;

	Lara.Inventory.OldBusy = Lara.Inventory.IsBusy;

	Rings[(int)RingTypes::Inventory] = &PCRing1;
	Rings[(int)RingTypes::Ammo] = &PCRing2;
	g_Renderer.DumpGameScene();

	if (resetMode)
		InvMode = InventoryMode::InGame;

	InitialiseInventory();
	Camera.numberFrames = 2;

	bool exitLoop = false;
	while (!exitLoop)
	{
		OBJLIST_SPACING = PHD_CENTER_X >> 1;

		if (CompassNeedleAngle != 1024)
			CompassNeedleAngle -= 32;

		UpdateInput();
		GameTimer++;

		if (IsClicked(In::Option))
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			exitLoop = true;
		}

		if (ThreadEnded)
			return true;

		DrawInventory();
		DrawCompass();

		switch (InvMode)
		{
		case InventoryMode::InGame:
			DoInventory();
			break;

		case InventoryMode::Statistics:
			DoStatisticsMode();
			break;

		case InventoryMode::Examine:
			DoExamineMode();
			break;

		case InventoryMode::Diary:
			DoDiary();
			break;

		case InventoryMode::Load:
			switch (DoLoad())
			{
			case LoadResult::Load:
				returnValue = true;
				exitLoop = true;
				break;

			case LoadResult::Cancel:
				exitLoop = !resetMode;

				if (resetMode)
					InvMode = InventoryMode::InGame;

				break;

			case LoadResult::None:
				break;
			}

			break;

		case InventoryMode::Save:
			if (DoSave())
			{
				exitLoop = !resetMode;
				if (resetMode)
					InvMode = InventoryMode::InGame;
			}

			break;
		}

		if (UseItem && NoInput())
			exitLoop = true;

		Camera.numberFrames = g_Renderer.SyncRenderer();
	}

	LastInvItem = Rings[(int)RingTypes::Inventory]->CurrentObjectList[Rings[(int)RingTypes::Inventory]->curobjinlist].invitem;
	UpdateWeaponStatus();

	if (UseItem)
		UseCurrentItem();

	Lara.Inventory.IsBusy = Lara.Inventory.OldBusy;
	InvMode = InventoryMode::None;

	return returnValue;
}

void GuiController::DoStatisticsMode()
{
	InvMode = InventoryMode::Statistics;

	if (GUI_INPUT_DESELECT)
	{
		SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
		InvMode = InventoryMode::InGame;
	}
}

void GuiController::DoExamineMode()
{
	InvMode = InventoryMode::Examine;

	if (GUI_INPUT_DESELECT)
	{
		SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
		InvMode = InventoryMode::None;
	}
}

void GuiController::DrawCompass()
{
	// TODO
	return;

	g_Renderer.DrawObjectOn2DPosition(130, 480, ID_COMPASS_ITEM, ANGLE(90.0f), 0, ANGLE(180.0f), InventoryObjectTable[INV_OBJECT_COMPASS].scale1);
	short compassSpeed = phd_sin(CompassNeedleAngle - LaraItem->Pose.Orientation.y);
	short compassAngle = (LaraItem->Pose.Orientation.y + compassSpeed) - ANGLE(180.0f);
	Matrix::CreateRotationY(compassAngle);
}

void GuiController::DoDiary()
{
	InvMode = InventoryMode::Diary;

	if (GUI_INPUT_PULSE_RIGHT &&
		Lara.Inventory.Diary.currentPage < Lara.Inventory.Diary.numPages)
	{
		Lara.Inventory.Diary.currentPage++;
		SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
	}

	if (GUI_INPUT_PULSE_LEFT &&
		Lara.Inventory.Diary.currentPage > 1)
	{
		Lara.Inventory.Diary.currentPage--;
		SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
	}

	if (GUI_INPUT_DESELECT)
	{
		SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
		InvMode = InventoryMode::None;
	}
}

short GuiController::GetLoadSaveSelection()
{
	return SelectedSaveSlot;
}

LoadResult GuiController::DoLoad()
{
	if (GUI_INPUT_PULSE_DOWN)
	{
		SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

		if (SelectedSaveSlot == SAVEGAME_MAX - 1)
			SelectedSaveSlot -= SAVEGAME_MAX - 1;
		else
			SelectedSaveSlot++;
	}

	if (GUI_INPUT_PULSE_UP)
	{
		SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

		if (SelectedSaveSlot == 0)
			SelectedSaveSlot += SAVEGAME_MAX - 1;
		else
			SelectedSaveSlot--;
	}

	if (GUI_INPUT_SELECT)
	{
		if (!SavegameInfos[SelectedSaveSlot].Present)
			SayNo();
		else
		{
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			g_GameFlow->SelectedSaveGame = SelectedSaveSlot;
			return LoadResult::Load;
		}
	}

	if (GUI_INPUT_DESELECT || IsClicked(In::Load))
		return LoadResult::Cancel;

	return LoadResult::None;
}

bool GuiController::DoSave()
{
	if (GUI_INPUT_PULSE_DOWN)
	{
		SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

		if (SelectedSaveSlot == SAVEGAME_MAX - 1)
			SelectedSaveSlot -= SAVEGAME_MAX - 1;
		else
			SelectedSaveSlot++;
	}

	if (GUI_INPUT_PULSE_UP)
	{
		SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

		if (SelectedSaveSlot == 0)
			SelectedSaveSlot += SAVEGAME_MAX - 1;
		else
			SelectedSaveSlot--;
	}

	if (GUI_INPUT_SELECT)
	{
		SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		SaveGame::Save(SelectedSaveSlot);
		g_GameScript->OnSave();
		return true;
	}

	if (GUI_INPUT_DESELECT || IsClicked(In::Save))
		return true;

	return false;
}

bool GuiController::PerformWaterskinCombine(int flag)
{
	int smallLiters = Lara.Inventory.SmallWaterskin - 1; // How many liters in the small one?
	int bigLiters = Lara.Inventory.BigWaterskin - 1;	 // How many liters in the big one?
	int smallCapacity = 3 - smallLiters;				 // How many more liters can fit in the small one?
	int bigCapacity = 5 - bigLiters;					 // How many more liters can fit in the big one?

	int i;
	if (flag)
	{
		// Big one isn't empty and the small one isn't full.
		if (Lara.Inventory.BigWaterskin != 1 && smallCapacity)
		{
			i = bigLiters;

			do
			{
				if (smallCapacity)
				{
					smallLiters++;
					smallCapacity--;
					bigLiters--;
				}

				i--;

			} while (i);

			Lara.Inventory.SmallWaterskin = smallLiters + 1;
			Lara.Inventory.BigWaterskin = bigLiters + 1;
			CombineObject1 = (smallLiters + 1) + (INV_OBJECT_SMALL_WATERSKIN - 1);
			return true;
		}
	}
	else 
	{
		// Small one isn't empty and the big one isn't full.
		if (Lara.Inventory.SmallWaterskin != 1 && bigCapacity)
		{
			i = Lara.Inventory.SmallWaterskin - 1;

			do
			{
				if (bigCapacity)
				{
					bigLiters++;
					bigCapacity--;
					smallLiters--;
				}

				i--;

			} while (i);

			Lara.Inventory.SmallWaterskin = smallLiters + 1;
			Lara.Inventory.BigWaterskin = bigLiters + 1;
			CombineObject1 = (bigLiters + 1) + (INV_OBJECT_BIG_WATERSKIN - 1);
			return true;
		}
	}

	return false;
}
