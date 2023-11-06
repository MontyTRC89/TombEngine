#include "framework.h"
#include "Game/Gui.h"

#include <OISKeyboard.h>

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/pickup/pickup.h"
#include "Game/savegame.h"
#include "Game/spotcam.h"
#include "Renderer/Renderer11.h"
#include "Scripting/Include/ScriptInterfaceGame.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/Input/InputAction.h"
#include "Specific/clock.h"
#include "Specific/configuration.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Input;
using namespace TEN::Renderer;
using namespace TEN::Utils;

namespace TEN::Gui
{
	constexpr int LINE_HEIGHT	  = 25;
	constexpr int PHD_CENTER_X	  = SCREEN_SPACE_RES.x / 2;
	constexpr int PHD_CENTER_Y	  = SCREEN_SPACE_RES.y / 2;
	constexpr int OBJLIST_SPACING = PHD_CENTER_X / 2;

	constexpr auto VOLUME_MAX			 = 100;
	constexpr auto VOLUME_STEP			 = VOLUME_MAX / 20;
	constexpr auto MOUSE_SENSITIVITY_MAX = 35;
	constexpr auto MOUSE_SENSITIVITY_MIN = 1;
	constexpr auto MOUSE_SMOOTHING_MAX	 = 5;
	constexpr auto MOUSE_SMOOTHING_MIN	 = 0;

	GuiController g_Gui;

	std::vector<std::string> OptionStrings =
	{
		STRING_USE,
		STRING_CHOOSE_AMMO,
		STRING_COMBINE,
		STRING_SEPARATE,
		STRING_EQUIP,
		STRING_COMBINE_WITH,
		STRING_LOAD_GAME,
		STRING_SAVE_GAME,
		STRING_EXAMINE,
		STRING_VIEW,
		STRING_CHOOSE_WEAPON,
		""
	//	STRING_READ_DIARY
	};

	std::vector<std::string> GeneralActionStrings =
	{
		STRING_ACTIONS_FORWARD,
		STRING_ACTIONS_BACKWARD,
		STRING_ACTIONS_LEFT,
		STRING_ACTIONS_RIGHT,
		STRING_ACTIONS_STEP_LEFT,
		STRING_ACTIONS_STEP_RIGHT,
		STRING_ACTIONS_WALK,
		STRING_ACTIONS_SPRINT,
		STRING_ACTIONS_CROUCH,
		STRING_ACTIONS_JUMP,
		STRING_ACTIONS_ROLL,
		STRING_ACTIONS_ACTION,
		STRING_ACTIONS_DRAW,
		STRING_ACTIONS_LOOK
	};

	std::vector<std::string> VehicleActionStrings =
	{
		STRING_ACTIONS_ACCELERATE,
		STRING_ACTIONS_REVERSE,
		STRING_ACTIONS_SPEED,
		STRING_ACTIONS_SLOW,
		STRING_ACTIONS_BRAKE,
		STRING_ACTIONS_FIRE
	};

	std::vector<std::string> QuickActionStrings =
	{
		STRING_ACTIONS_FLARE,
		STRING_ACTIONS_SMALL_MEDIPACK,
		STRING_ACTIONS_LARGE_MEDIPACK,
		STRING_ACTIONS_PREVIOUS_WEAPON,
		STRING_ACTIONS_NEXT_WEAPON,
		STRING_ACTIONS_WEAPON_1,
		STRING_ACTIONS_WEAPON_2,
		STRING_ACTIONS_WEAPON_3,
		STRING_ACTIONS_WEAPON_4,
		STRING_ACTIONS_WEAPON_5,
		STRING_ACTIONS_WEAPON_6,
		STRING_ACTIONS_WEAPON_7,
		STRING_ACTIONS_WEAPON_8,
		STRING_ACTIONS_WEAPON_9,
		STRING_ACTIONS_WEAPON_10
	};

	std::vector<std::string> MenuActionStrings =
	{
		STRING_ACTIONS_SELECT,
		STRING_ACTIONS_DESELECT,
		STRING_ACTIONS_PAUSE,
		STRING_ACTIONS_INVENTORY,
		STRING_ACTIONS_SAVE,
		STRING_ACTIONS_LOAD
	};

	bool GuiController::GuiIsPulsed(ActionID actionID) const
	{
		constexpr auto DELAY		 = 0.1f;
		constexpr auto INITIAL_DELAY = 0.4f;

		// Action already held prior to entering menu; lock input.
		if (GetActionTimeActive(actionID) >= TimeInMenu)
			return false;

		// Pulse only directional inputs.
		auto oppositeAction = std::optional<ActionID>(std::nullopt);
		switch (actionID)
		{
		case In::Forward:
			oppositeAction = In::Back;
			break;

		case In::Back:
			oppositeAction = In::Forward;
			break;

		case In::Left:
			oppositeAction = In::Right;
			break;

		case In::Right:
			oppositeAction = In::Left;
			break;

		default:
			break;
		}

		// Opposite action held; lock input.
		bool isActionLocked = oppositeAction.has_value() ? IsHeld(*oppositeAction) : false;
		if (isActionLocked)
			return false;

		return IsPulsed(actionID, DELAY, INITIAL_DELAY);
	}

	bool GuiController::GuiIsSelected(bool onClicked) const
	{
		if (onClicked)
		{
			return ((IsClicked(In::Select) || IsClicked(In::Action)) && CanSelect());
		}
		else
		{
			return ((IsReleased(In::Select) || IsReleased(In::Action)) && CanSelect());
		}
	}
	
	bool GuiController::GuiIsDeselected() const
	{
		return ((IsClicked(In::Deselect) || IsClicked(In::Draw)) && CanDeselect());
	}

	bool GuiController::CanSelect() const
	{
		// Holding Deselect safely cancels select actions.
		if (IsHeld(In::Deselect))
			return false;

		// Avoid Select or Action release interference when entering inventory.
		if (GetActionTimeActive(In::Select) < TimeInMenu && GetActionTimeActive(In::Action) < TimeInMenu)
			return true;

		return false;
	}

	bool GuiController::CanDeselect() const
	{
		return !(IsHeld(In::Select) || IsHeld(In::Action));
	}

	SettingsData& GuiController::GetCurrentSettings()
	{
		return CurrentSettings;
	}

	const InventoryRing& GuiController::GetRing(RingTypes ringType)
	{
		return Rings[(int)ringType];
	}

	int GuiController::GetSelectedOption()
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
		if (mode != InvMode)
		{
			TimeInMenu = 0.0f;
			InvMode = mode;
		}
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

	void GuiController::SetLastInventoryItem(int itemNumber)
	{
		LastInvItem = itemNumber;
	}

	void GuiController::DrawInventory()
	{
		g_Renderer.RenderInventory();
	}

	InventoryResult GuiController::TitleOptions(ItemInfo* item)
	{
		enum TitleOption
		{
			NewGame,
			LoadGame,
			Options,
			ExitGame
		};

		static const int numTitleOptions	= 3;
		static const int numLoadGameOptions = SAVEGAME_MAX - 1;
		static const int numOptionsOptions	= 2;

		static int selectedOptionBackup;
		auto inventoryResult = InventoryResult::None;

		TimeInMenu++;

		// Stuff for credits goes here!

		switch (MenuToDisplay)
		{
		case Menu::Title:
			OptionCount = g_GameFlow->IsLoadSaveEnabled() ? numTitleOptions : (numTitleOptions - 1);
			break;

		case Menu::SelectLevel:
			inventoryResult = InventoryResult::None;
			OptionCount = g_GameFlow->GetNumLevels() - 2;
			break;

		case Menu::LoadGame:
			OptionCount = numLoadGameOptions;
			break;

		case Menu::Options:
			OptionCount = numOptionsOptions;
			break;

		case Menu::Display:
			HandleDisplaySettingsInput(false);
			return inventoryResult;

		case Menu::GeneralActions:
		case Menu::VehicleActions:
		case Menu::QuickActions:
		case Menu::MenuActions:
			HandleControlSettingsInput(item, false);
			return inventoryResult;

		case Menu::OtherSettings:
			HandleOtherSettingsInput(false);
			return inventoryResult;
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
			if (GuiIsPulsed(In::Forward))
			{
				SelectedOption = (SelectedOption <= 0) ? OptionCount : (SelectedOption - 1);
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}

			if (GuiIsPulsed(In::Back))
			{
				if (SelectedOption < OptionCount)
					SelectedOption++;
				else
					SelectedOption -= OptionCount;

				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}

			if (GuiIsDeselected() && MenuToDisplay != Menu::Title)
			{
				MenuToDisplay = Menu::Title;
				SelectedOption = selectedOptionBackup;
				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			}

			if (GuiIsSelected())
			{
				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

				if (MenuToDisplay == Menu::Title)
				{
					// Skip load game entry if loading and saving is disabled.
					int realSelectedOption = SelectedOption;
					if (!g_GameFlow->IsLoadSaveEnabled() && SelectedOption > TitleOption::NewGame)
						realSelectedOption++;

					switch (realSelectedOption)
					{
					case TitleOption::NewGame:
						if (g_GameFlow->IsLevelSelectEnabled())
						{
							selectedOptionBackup = SelectedOption;
							SelectedOption = 0;
							MenuToDisplay = Menu::SelectLevel;
						}
						else
						{
							inventoryResult = InventoryResult::NewGame;
						}

						break;

					case TitleOption::LoadGame:
						selectedOptionBackup = SelectedOption;
						SelectedOption = 0;
						MenuToDisplay = Menu::LoadGame;
						SaveGame::LoadSavegameInfos();
						break;

					case TitleOption::Options:
						selectedOptionBackup = SelectedOption;
						SelectedOption = 0;
						MenuToDisplay = Menu::Options;
						break;

					case TitleOption::ExitGame:
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
			if (screenResolution.x == CurrentSettings.Configuration.ScreenWidth &&
				screenResolution.y == CurrentSettings.Configuration.ScreenHeight)
			{
				CurrentSettings.SelectedScreenResolution = i;
				break;
			}
		}
	}

	void GuiController::HandleDisplaySettingsInput(bool fromPauseMenu)
	{
		enum DisplaySettingsOption
		{
			ScreenResolution,
			Windowed,
			ShadowType,
			Caustics,
			Antialiasing,
			Save,
			Cancel
		};

		static const int numDisplaySettingsOptions = 6;

		OptionCount = numDisplaySettingsOptions;

		if (GuiIsDeselected())
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr);
			MenuToDisplay = Menu::Options;
			SelectedOption = 0;
			return;
		}

		if (GuiIsPulsed(In::Left))
		{
			switch (SelectedOption)
			{
			case DisplaySettingsOption::ScreenResolution:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				if (CurrentSettings.SelectedScreenResolution > 0)
					CurrentSettings.SelectedScreenResolution--;

				break;

			case DisplaySettingsOption::Windowed:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				CurrentSettings.Configuration.EnableWindowedMode = !CurrentSettings.Configuration.EnableWindowedMode;
				break;

			case DisplaySettingsOption::ShadowType:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

				if (CurrentSettings.Configuration.ShadowType == ShadowMode::None)
					CurrentSettings.Configuration.ShadowType = ShadowMode::All;
				else
					CurrentSettings.Configuration.ShadowType = ShadowMode(int(CurrentSettings.Configuration.ShadowType) - 1);

				break;

			case DisplaySettingsOption::Caustics:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				CurrentSettings.Configuration.EnableCaustics = !CurrentSettings.Configuration.EnableCaustics;
				break;

			case DisplaySettingsOption::Antialiasing:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

				if (CurrentSettings.Configuration.AntialiasingMode == AntialiasingMode::None)
					CurrentSettings.Configuration.AntialiasingMode = AntialiasingMode::High;
				else
					CurrentSettings.Configuration.AntialiasingMode = AntialiasingMode(int(CurrentSettings.Configuration.AntialiasingMode) - 1);

				break;
			}
		}

		if (GuiIsPulsed(In::Right))
		{
			switch (SelectedOption)
			{
			case DisplaySettingsOption::ScreenResolution:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				if (CurrentSettings.SelectedScreenResolution < g_Configuration.SupportedScreenResolutions.size() - 1)
					CurrentSettings.SelectedScreenResolution++;
				break;

			case DisplaySettingsOption::Windowed:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				CurrentSettings.Configuration.EnableWindowedMode = !CurrentSettings.Configuration.EnableWindowedMode;
				break;

			case DisplaySettingsOption::ShadowType:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				if (CurrentSettings.Configuration.ShadowType == ShadowMode::All)
					CurrentSettings.Configuration.ShadowType = ShadowMode::None;
				else
					CurrentSettings.Configuration.ShadowType = ShadowMode(int(CurrentSettings.Configuration.ShadowType) + 1);

				break;

			case DisplaySettingsOption::Caustics:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				CurrentSettings.Configuration.EnableCaustics = !CurrentSettings.Configuration.EnableCaustics;
				break;

			case DisplaySettingsOption::Antialiasing:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

				if (CurrentSettings.Configuration.AntialiasingMode == AntialiasingMode::High)
					CurrentSettings.Configuration.AntialiasingMode = AntialiasingMode::None;
				else
					CurrentSettings.Configuration.AntialiasingMode = AntialiasingMode(int(CurrentSettings.Configuration.AntialiasingMode) + 1);

				break;
			}
		}

		if (GuiIsPulsed(In::Forward))
		{
			if (SelectedOption <= 0)
				SelectedOption += OptionCount;
			else
				SelectedOption--;

			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}

		if (GuiIsPulsed(In::Back))
		{
			if (SelectedOption < OptionCount)
				SelectedOption++;
			else
				SelectedOption -= OptionCount;

			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}

		if (GuiIsSelected())
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

			if (SelectedOption == DisplaySettingsOption::Save)
			{
				// Save the configuration.
				auto screenResolution = g_Configuration.SupportedScreenResolutions[CurrentSettings.SelectedScreenResolution];
				CurrentSettings.Configuration.ScreenWidth = screenResolution.x;
				CurrentSettings.Configuration.ScreenHeight = screenResolution.y;

				g_Configuration = CurrentSettings.Configuration;
				SaveConfiguration();

				// Reset screen and go back.
				g_Renderer.ChangeScreenResolution(CurrentSettings.Configuration.ScreenWidth, CurrentSettings.Configuration.ScreenHeight, 
					CurrentSettings.Configuration.EnableWindowedMode);

				MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
				SelectedOption = fromPauseMenu ? 1 : 0;
			}
			else if (SelectedOption == DisplaySettingsOption::Cancel)
			{
				MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
				SelectedOption = fromPauseMenu ? 1 : 0;
			}
		}
	}

	void GuiController::HandleControlSettingsInput(ItemInfo* item, bool fromPauseMenu)
	{
		unsigned int numControlSettingsOptions = 0;
		switch (MenuToDisplay)
		{
		default:
		case Menu::GeneralActions:
			numControlSettingsOptions = (int)GeneralActionStrings.size() + 2;
			break;

		case Menu::VehicleActions:
			numControlSettingsOptions = (int)VehicleActionStrings.size() + 2;
			break;

		case Menu::QuickActions:
			numControlSettingsOptions = (int)QuickActionStrings.size() + 2;
			break;

		case Menu::MenuActions:
			numControlSettingsOptions = (int)MenuActionStrings.size() + 2;
			break;
		}

		OptionCount = numControlSettingsOptions;
		CurrentSettings.NewKeyWaitTimer = 0.0f;

		if (CurrentSettings.IgnoreInput)
		{
			if (NoAction())
				CurrentSettings.IgnoreInput = false;

			return;
		}

		if (GuiIsSelected() &&
			SelectedOption <= (numControlSettingsOptions - 3))
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			CurrentSettings.NewKeyWaitTimer = SettingsData::NEW_KEY_WAIT_TIMEOUT;
			CurrentSettings.IgnoreInput = true;
		}

		if (CurrentSettings.NewKeyWaitTimer > 0.0f)
		{
			ClearAllActions();

			while (CurrentSettings.NewKeyWaitTimer > 0.0f)
			{
				CurrentSettings.NewKeyWaitTimer -= 1.0f;
				if (CurrentSettings.NewKeyWaitTimer <= 0.0f)
					CurrentSettings.NewKeyWaitTimer = 0.0f;

				UpdateInputActions(item);

				if (CurrentSettings.IgnoreInput)
				{
					if (NoAction())
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

					if (selectedKey && !g_KeyNames[selectedKey].empty())
					{
						unsigned int baseIndex = 0;
						switch (MenuToDisplay)
						{
						case Menu::VehicleActions:
							baseIndex = (unsigned int)GeneralActionStrings.size();
							break;

						case Menu::QuickActions:
							baseIndex = unsigned int(GeneralActionStrings.size() + VehicleActionStrings.size());
							break;

						case Menu::MenuActions:
							baseIndex = unsigned int(GeneralActionStrings.size() + VehicleActionStrings.size() + QuickActionStrings.size());
							break;

						default:
							break;
						}

						Bindings[1][baseIndex + SelectedOption] = selectedKey;
						DefaultConflict();

						CurrentSettings.NewKeyWaitTimer = 0.0f;
						CurrentSettings.IgnoreInput = true;
						return;
					}
				}

				if (fromPauseMenu)
				{
					g_Renderer.RenderInventory();
					Camera.numberFrames = g_Renderer.Synchronize();
				}
				else
				{
					g_Renderer.RenderTitle();
					Camera.numberFrames = g_Renderer.Synchronize();
					int numFrames = Camera.numberFrames;
					ControlPhase(numFrames);
				}
			}
		}
		else
		{
			if (GuiIsPulsed(In::Forward))
			{
				if (SelectedOption <= 0)
					SelectedOption += OptionCount;
				else
					SelectedOption--;

				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}

			if (GuiIsPulsed(In::Back))
			{
				if (SelectedOption < OptionCount)
					SelectedOption++;
				else
					SelectedOption -= OptionCount;

				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}

			// HACK: Menu screen scroll.
			if (GuiIsPulsed(In::Left) || GuiIsPulsed(In::Right))
			{
				auto menu = std::optional<Menu>(std::nullopt);

				if (GuiIsPulsed(In::Left))
				{
					if ((int)MenuToDisplay == (int)Menu::GeneralActions)
					{
						menu = Menu::MenuActions;
					}
					else
					{
						menu = Menu((int)MenuToDisplay - 1);
					}
				}
				else if (GuiIsPulsed(In::Right))
				{
					if ((int)MenuToDisplay == (int)Menu::MenuActions)
					{
						menu = Menu::GeneralActions;
					}
					else
					{
						menu = Menu((int)MenuToDisplay + 1);
					}
				}

				if (menu.has_value())
				{
					MenuToDisplay = *menu;
					SelectedOption = 0;
					SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
					return;
				}
			}

			if (GuiIsSelected())
			{
				// Defaults.
				if (SelectedOption == (OptionCount - 2))
				{
					SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
					ApplyDefaultBindings();
					return;
				}

				// Apply.
				if (SelectedOption == (OptionCount - 1))
				{
					SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
					CurrentSettings.Configuration.Bindings = Bindings[1];
					g_Configuration.Bindings = Bindings[1];
					SaveConfiguration();
					MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
					SelectedOption = 2;
					return;
				}

				// Cancel.
				if (SelectedOption == OptionCount)
				{
					SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
					Bindings[1] = CurrentSettings.Configuration.Bindings;
					MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
					SelectedOption = 2;
					return;
				}
			}

			if (GuiIsDeselected())
			{
				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

				MenuToDisplay = Menu::Options;
				SelectedOption = 2;
			}
		}
	}

	void GuiController::BackupOptions()
	{
		CurrentSettings.Configuration = g_Configuration;
	}

	void GuiController::HandleOptionsInput()
	{
		enum OptionsOption
		{
			Display,
			OtherSettings,
			Controls
		};

		switch (SelectedOption)
		{
		case OptionsOption::Display:
			FillDisplayOptions();
			MenuToDisplay = Menu::Display;
			SelectedOption = 0;
			break;

		case OptionsOption::OtherSettings:
			BackupOptions();
			MenuToDisplay = Menu::OtherSettings;
			SelectedOption = 0;
			break;

		case OptionsOption::Controls:
			BackupOptions();
			MenuToDisplay = Menu::GeneralActions;
			SelectedOption = 0;
			break;
		}
	}

	void GuiController::HandleOtherSettingsInput(bool fromPauseMenu)
	{
		enum OtherSettingsOption
		{
			Reverb,
			MusicVolume,
			SfxVolume,
			Subtitles,
			AutoTargeting,
			TargetHighlighter,
			ToggleRumble,
			ThumbstickCameraControl,
			MouseSensitivity,
			MouseSmoothing,

			Apply,
			Cancel
		};

		static const auto numOtherSettingsOptions = 11;

		OptionCount = numOtherSettingsOptions;

		if (GuiIsDeselected())
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

			MenuToDisplay = Menu::Options;
			SelectedOption = 1;

			SetVolumeTracks(g_Configuration.MusicVolume);
			SetVolumeFX(g_Configuration.SfxVolume);
			return;
		}

		if (GuiIsPulsed(In::Left) || GuiIsPulsed(In::Right))
		{
			switch (SelectedOption)
			{
			case OtherSettingsOption::Reverb:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				CurrentSettings.Configuration.EnableReverb = !CurrentSettings.Configuration.EnableReverb;
				break;

			case OtherSettingsOption::Subtitles:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				CurrentSettings.Configuration.EnableSubtitles = !CurrentSettings.Configuration.EnableSubtitles;
				break;

			case OtherSettingsOption::AutoTargeting:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				CurrentSettings.Configuration.EnableAutoTargeting = !CurrentSettings.Configuration.EnableAutoTargeting;
				break;

			case OtherSettingsOption::TargetHighlighter:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				CurrentSettings.Configuration.EnableTargetHighlighter = !CurrentSettings.Configuration.EnableTargetHighlighter;
				break;
				
			case OtherSettingsOption::ToggleRumble:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				CurrentSettings.Configuration.EnableRumble = !CurrentSettings.Configuration.EnableRumble;
				break;

			case OtherSettingsOption::ThumbstickCameraControl:
				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				CurrentSettings.Configuration.EnableThumbstickCamera = !CurrentSettings.Configuration.EnableThumbstickCamera;
				break;
			}
		}

		if (IsPulsed(In::Left, 0.05f, 0.4f))
		{
			bool isVolumeAdjusted = false;
			switch (SelectedOption)
			{
			case OtherSettingsOption::MusicVolume:
				if (CurrentSettings.Configuration.MusicVolume > 0)
				{
					CurrentSettings.Configuration.MusicVolume -= VOLUME_STEP;
					if (CurrentSettings.Configuration.MusicVolume < 0)
						CurrentSettings.Configuration.MusicVolume = 0;

					SetVolumeTracks(CurrentSettings.Configuration.MusicVolume);
					isVolumeAdjusted = true;
				}

				break;

			case OtherSettingsOption::SfxVolume:
				if (CurrentSettings.Configuration.SfxVolume > 0)
				{
					CurrentSettings.Configuration.SfxVolume -= VOLUME_STEP;
					if (CurrentSettings.Configuration.SfxVolume < 0)
						CurrentSettings.Configuration.SfxVolume = 0;

					SetVolumeFX(CurrentSettings.Configuration.SfxVolume);
					isVolumeAdjusted = true;
				}

				break;

			case OtherSettingsOption::MouseSensitivity:
				if (CurrentSettings.Configuration.MouseSensitivity > MOUSE_SENSITIVITY_MIN)
				{
					CurrentSettings.Configuration.MouseSensitivity -= 1;
					if (CurrentSettings.Configuration.MouseSensitivity < MOUSE_SENSITIVITY_MIN)
						CurrentSettings.Configuration.MouseSensitivity = MOUSE_SENSITIVITY_MIN;

					SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				}

				break;

			case OtherSettingsOption::MouseSmoothing:
				if (CurrentSettings.Configuration.MouseSmoothing > MOUSE_SMOOTHING_MIN)
				{
					CurrentSettings.Configuration.MouseSmoothing -= 1;
					if (CurrentSettings.Configuration.MouseSmoothing < MOUSE_SMOOTHING_MIN)
						CurrentSettings.Configuration.MouseSmoothing = MOUSE_SMOOTHING_MIN;

					SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				}

				break;
			}

			if (isVolumeAdjusted)
			{
				if (IsClicked(In::Left))
					SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}
		}

		if (IsPulsed(In::Right, 0.05f, 0.4f))
		{
			bool isVolumeAdjusted = false;
			switch (SelectedOption)
			{
			case OtherSettingsOption::MusicVolume:
				if (CurrentSettings.Configuration.MusicVolume < VOLUME_MAX)
				{
					CurrentSettings.Configuration.MusicVolume += VOLUME_STEP;
					if (CurrentSettings.Configuration.MusicVolume > VOLUME_MAX)
						CurrentSettings.Configuration.MusicVolume = VOLUME_MAX;

					SetVolumeTracks(CurrentSettings.Configuration.MusicVolume);
					isVolumeAdjusted = true;
				}

				break;

			case OtherSettingsOption::SfxVolume:
				if (CurrentSettings.Configuration.SfxVolume < VOLUME_MAX)
				{
					CurrentSettings.Configuration.SfxVolume += VOLUME_STEP;
					if (CurrentSettings.Configuration.SfxVolume > VOLUME_MAX)
						CurrentSettings.Configuration.SfxVolume = VOLUME_MAX;

					SetVolumeFX(CurrentSettings.Configuration.SfxVolume);
					isVolumeAdjusted = true;
				}

				break;

			case OtherSettingsOption::MouseSensitivity:
				if (CurrentSettings.Configuration.MouseSensitivity < MOUSE_SENSITIVITY_MAX)
				{
					CurrentSettings.Configuration.MouseSensitivity += 1;
					if (CurrentSettings.Configuration.MouseSensitivity > MOUSE_SENSITIVITY_MAX)
						CurrentSettings.Configuration.MouseSensitivity = MOUSE_SENSITIVITY_MAX;

					SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				}

				break;

			case OtherSettingsOption::MouseSmoothing:
				if (CurrentSettings.Configuration.MouseSmoothing < MOUSE_SMOOTHING_MAX)
				{
					CurrentSettings.Configuration.MouseSmoothing += 1;
					if (CurrentSettings.Configuration.MouseSmoothing > MOUSE_SMOOTHING_MAX)
						CurrentSettings.Configuration.MouseSmoothing = MOUSE_SMOOTHING_MAX;

					SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
				}

				break;
			}

			if (isVolumeAdjusted)
			{
				if (IsClicked(In::Right))
					SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}
		}

		if (GuiIsPulsed(In::Forward))
		{
			if (SelectedOption <= 0)
			{
				SelectedOption += OptionCount;
			}
			else
			{
				SelectedOption--;
			}

			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}

		if (GuiIsPulsed(In::Back))
		{
			if (SelectedOption < OptionCount)
			{
				SelectedOption++;
			}
			else
			{
				SelectedOption -= OptionCount;
			}

			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}

		if (GuiIsSelected())
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

			if (SelectedOption == OtherSettingsOption::Apply)
			{
				// Was rumble setting changed?
				bool indicateRumble = CurrentSettings.Configuration.EnableRumble && !g_Configuration.EnableRumble;

				// Save the configuration.
				g_Configuration = CurrentSettings.Configuration;
				SaveConfiguration();

				// Rumble if setting was changed.
				if (indicateRumble)
					Rumble(0.5f);

				MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
				SelectedOption = 1;
			}
			else if (SelectedOption == OtherSettingsOption::Cancel)
			{
				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
				SetVolumeTracks(g_Configuration.MusicVolume);
				SetVolumeFX(g_Configuration.SfxVolume);
				MenuToDisplay = fromPauseMenu ? Menu::Pause : Menu::Options;
				SelectedOption = 1;
			}
		}
	}

	InventoryResult GuiController::DoPauseMenu(ItemInfo* item)
	{
		enum PauseMenuOption
		{
			Statistics,
			Options,
			ExitToTitle
		};

		static const int numPauseOptions	  = 2;
		static const int numStatisticsOptions = 0;
		static const int numOptionsOptions	  = 2;

		TimeInMenu++;
		UpdateInputActions(item);

		switch (MenuToDisplay)
		{
		case Menu::Pause:
			OptionCount = numPauseOptions;
			break;

		case Menu::Statistics:
			OptionCount = numStatisticsOptions;
			break;

		case Menu::Options:
			OptionCount = numOptionsOptions;
			break;

		case Menu::Display:
			HandleDisplaySettingsInput(true);
			return InventoryResult::None;

		case Menu::GeneralActions:
		case Menu::VehicleActions:
		case Menu::QuickActions:
		case Menu::MenuActions:
			HandleControlSettingsInput(item, true);
			return InventoryResult::None;

		case Menu::OtherSettings:
			HandleOtherSettingsInput(true);
			return InventoryResult::None;
		}

		if (MenuToDisplay == Menu::Pause ||
			MenuToDisplay == Menu::Options)
		{
			if (GuiIsPulsed(In::Forward))
			{
				if (SelectedOption <= 0)
					SelectedOption += OptionCount;
				else
					SelectedOption--;

				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}

			if (GuiIsPulsed(In::Back))
			{
				if (SelectedOption < OptionCount)
					SelectedOption++;
				else
					SelectedOption -= OptionCount;

				SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			}
		}

		if (GuiIsDeselected() || IsClicked(In::Pause))
		{
			if (MenuToDisplay == Menu::Pause)
			{
				SetInventoryMode(InventoryMode::None);
				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
				return InventoryResult::None;
			}

			if (MenuToDisplay == Menu::Statistics ||
				MenuToDisplay == Menu::Options)
			{
				SelectedOption = ((MenuToDisplay == Menu::Statistics) ? PauseMenuOption::Statistics : PauseMenuOption::Options);
				MenuToDisplay = Menu::Pause;
				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			}
		}

		if (GuiIsSelected())
		{
			switch (MenuToDisplay)
			{
			case Menu::Pause:

				switch (SelectedOption)
				{
				case PauseMenuOption::Statistics:
					SelectedOption = 0;
					MenuToDisplay = Menu::Statistics;
					break;

				case PauseMenuOption::Options:
					SelectedOption = 0;
					MenuToDisplay = Menu::Options;
					break;

				case PauseMenuOption::ExitToTitle:
					SetInventoryMode(InventoryMode::None);
					return InventoryResult::ExitToTitle;
					break;
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
			if (CombineTable[n].Item1 == objectNumber1 &&
				CombineTable[n].Item2 == objectNumber2)
				return true;

			if (CombineTable[n].Item1 == objectNumber2 &&
				CombineTable[n].Item2 == objectNumber1)
				return true;
		}

		return false;
	}

	bool GuiController::IsItemCurrentlyCombinable(int objectNumber)
	{
		static const int numSmallWaterskins = INV_OBJECT_SMALL_WATERSKIN_3L - INV_OBJECT_SMALL_WATERSKIN_EMPTY + 1;
		static const int numBigWaterskins	= INV_OBJECT_BIG_WATERSKIN_5L - INV_OBJECT_BIG_WATERSKIN_EMPTY + 1;

		if (objectNumber < INV_OBJECT_SMALL_WATERSKIN_EMPTY || objectNumber > INV_OBJECT_BIG_WATERSKIN_5L)//trash
		{
			for (int n = 0; n < MAX_COMBINES; n++)
			{
				if (CombineTable[n].Item1 == objectNumber)
				{
					if (IsItemInInventory(CombineTable[n].Item2))
						return true;
				}

				if (CombineTable[n].Item2 == objectNumber)
				{
					if (IsItemInInventory(CombineTable[n].Item1))
						return true;
				}
			}
		}
		else if (objectNumber > INV_OBJECT_SMALL_WATERSKIN_3L)
		{
			for (int n = 0; n < numSmallWaterskins; n++)
			{
				if (IsItemInInventory(n + INV_OBJECT_SMALL_WATERSKIN_EMPTY))
					return true;
			}
		}
		else
		{
			for (int n = 0; n < numBigWaterskins; n++)
			{
				if (IsItemInInventory(n + INV_OBJECT_BIG_WATERSKIN_EMPTY))
					return true;
			}
		}

		return false;
	}

	bool GuiController::IsItemInInventory(int objectNumber)
	{
		const auto& ring = Rings[(int)RingTypes::Inventory];
		for (int i = 0; i < INVENTORY_TABLE_SIZE; i++)
		{
			const auto& listObject = ring.CurrentObjectList[i];
			if (listObject.InventoryItem == objectNumber)
				return true;
		}

		return false;
	}

	void GuiController::CombineObjects(ItemInfo* item, int objectNumber1, int objectNumber2)
	{
		int n;
		for (n = 0; n < MAX_COMBINES; n++)
		{
			if (CombineTable[n].Item1 == objectNumber1 &&
				CombineTable[n].Item2 == objectNumber2)
				break;

			if (CombineTable[n].Item1 == objectNumber2 &&
				CombineTable[n].Item2 == objectNumber1)
				break;
		}

		CombineTable[n].CombineRoutine(item, false);
		ConstructObjectList(item);
		SetupObjectListStartPosition(CombineTable[n].CombinedItem);
		HandleObjectChangeover((int)RingTypes::Inventory);
	}

	void GuiController::SeparateObject(ItemInfo* item, int objectNumber)
	{
		int n = 0;
		for (n = 0; n < MAX_COMBINES; n++)
		{
			if (CombineTable[n].CombinedItem == objectNumber)
				break;
		}

		CombineTable[n].CombineRoutine(item, true);
		ConstructObjectList(item);
		SetupObjectListStartPosition(CombineTable[n].Item1);
	}

	void GuiController::SetupObjectListStartPosition(int newObjectNumber)
	{
		auto& ring = Rings[(int)RingTypes::Inventory];
		for (int i = 0; i < INVENTORY_TABLE_SIZE; i++)
		{
			const auto& listObject = ring.CurrentObjectList[i];
			if (listObject.InventoryItem == newObjectNumber)
				ring.CurrentObjectInList = i;
		}
	}

	void GuiController::HandleObjectChangeover(int ringIndex)
	{
		CurrentSelectedOption = 0;
		MenuActive = true;
		SetupAmmoSelector();
	}

	void GuiController::SetupAmmoSelector()
	{
		const auto& ring = Rings[(int)RingTypes::Inventory];
		const auto& invObject = InventoryObjectTable[ring.CurrentObjectList[ring.CurrentObjectInList].InventoryItem];

		int number = 0;
		unsigned __int64 options = invObject.Options;
		AmmoSelectorFlag = 0;
		NumAmmoSlots = 0;

		if (Rings[(int)RingTypes::Ammo].RingActive)
			return;
	
		AmmoObjectList[0].Orientation = EulerAngles::Zero;
		AmmoObjectList[1].Orientation = EulerAngles::Zero;
		AmmoObjectList[2].Orientation = EulerAngles::Zero;

		if (options &
			(OPT_CHOOSE_AMMO_UZI | OPT_CHOOSE_AMMO_PISTOLS | OPT_CHOOSE_AMMO_REVOLVER | OPT_CHOOSE_AMMO_CROSSBOW |
			OPT_CHOOSE_AMMO_HK | OPT_CHOOSE_AMMO_SHOTGUN | OPT_CHOOSE_AMMO_GRENADEGUN | OPT_CHOOSE_AMMO_HARPOON | OPT_CHOOSE_AMMO_ROCKET))
		{
			AmmoSelectorFlag = 1;
			AmmoSelectorFadeDir = 1;

			if (options & OPT_CHOOSE_AMMO_UZI)
			{
				AmmoObjectList[0].InventoryItem = INV_OBJECT_UZI_AMMO;
				AmmoObjectList[0].Amount = Ammo.AmountUziAmmo;
				number++;
				NumAmmoSlots = number;
				CurrentAmmoType = &Ammo.CurrentUziAmmoType;
			}

			if (options & OPT_CHOOSE_AMMO_PISTOLS)
			{
				number++;
				AmmoObjectList[0].InventoryItem = INV_OBJECT_PISTOLS_AMMO;
				AmmoObjectList[0].Amount = Ammo.AmountPistolsAmmo;
				NumAmmoSlots = number;
				CurrentAmmoType = &Ammo.CurrentPistolsAmmoType;
			}

			if (options & OPT_CHOOSE_AMMO_REVOLVER)
			{
				number++;
				AmmoObjectList[0].InventoryItem = INV_OBJECT_REVOLVER_AMMO;
				AmmoObjectList[0].Amount = Ammo.AmountRevolverAmmo;
				NumAmmoSlots = number;
				CurrentAmmoType = &Ammo.CurrentRevolverAmmoType;
			}

			if (options & OPT_CHOOSE_AMMO_CROSSBOW)
			{
				AmmoObjectList[number].InventoryItem = INV_OBJECT_CROSSBOW_AMMO_1;
				AmmoObjectList[number].Amount = Ammo.AmountCrossBowAmmo1;
				number++;
				AmmoObjectList[number].InventoryItem = INV_OBJECT_CROSSBOW_AMMO_2;
				AmmoObjectList[number].Amount = Ammo.AmountCrossBowAmmo2;
				number++;
				AmmoObjectList[number].InventoryItem = INV_OBJECT_CROSSBOW_AMMO_3;
				AmmoObjectList[number].Amount = Ammo.AmountCrossBowAmmo3;
				number++;
				NumAmmoSlots = number;
				CurrentAmmoType = &Ammo.CurrentCrossBowAmmoType;
			}

			if (options & OPT_CHOOSE_AMMO_HK)
			{
				AmmoObjectList[number].InventoryItem = INV_HK_MODE1;
				AmmoObjectList[number].Amount = Ammo.AmountHKAmmo1;
				number++;
				AmmoObjectList[number].InventoryItem = INV_HK_MODE2;
				AmmoObjectList[number].Amount = Ammo.AmountHKAmmo1;
				number++;
				AmmoObjectList[number].InventoryItem = INV_HK_MODE3;
				AmmoObjectList[number].Amount = Ammo.AmountHKAmmo1;
				number++;
				NumAmmoSlots = number;
				CurrentAmmoType = &Ammo.CurrentHKAmmoType;
			}

			if (options & OPT_CHOOSE_AMMO_SHOTGUN)
			{
				AmmoObjectList[number].InventoryItem = INV_OBJECT_SHOTGUN_AMMO_1;
				AmmoObjectList[number].Amount = Ammo.AmountShotGunAmmo1;
				number++;
				AmmoObjectList[number].InventoryItem = INV_OBJECT_SHOTGUN_AMMO_2;
				AmmoObjectList[number].Amount = Ammo.AmountShotGunAmmo2;
				number++;
				NumAmmoSlots = number;
				CurrentAmmoType = &Ammo.CurrentShotGunAmmoType;
			}

			if (options & OPT_CHOOSE_AMMO_GRENADEGUN)
			{
				AmmoObjectList[number].InventoryItem = INV_OBJECT_GRENADE_AMMO_1;
				AmmoObjectList[number].Amount = Ammo.AmountGrenadeAmmo1;
				number++;
				AmmoObjectList[number].InventoryItem = INV_OBJECT_GRENADE_AMMO_2;
				AmmoObjectList[number].Amount = Ammo.AmountGrenadeAmmo2;
				number++;
				AmmoObjectList[number].InventoryItem = INV_OBJECT_GRENADE_AMMO_3;
				AmmoObjectList[number].Amount = Ammo.AmountGrenadeAmmo3;
				number++;
				NumAmmoSlots = number;
				CurrentAmmoType = &Ammo.CurrentGrenadeGunAmmoType;
			}

			if (options & OPT_CHOOSE_AMMO_HARPOON)
			{
				AmmoObjectList[number].InventoryItem = INV_OBJECT_HARPOON_AMMO;
				AmmoObjectList[number].Amount = Ammo.AmountHarpoonAmmo;
				number++;
				NumAmmoSlots = number;
				CurrentAmmoType = &Ammo.CurrentHarpoonAmmoType;
			}

			if (options & OPT_CHOOSE_AMMO_ROCKET)
			{
				AmmoObjectList[number].InventoryItem = INV_OBJECT_ROCKET_AMMO;
				AmmoObjectList[number].Amount = Ammo.AmountRocketsAmmo;
				number++;
				NumAmmoSlots = number;
				CurrentAmmoType = &Ammo.CurrentRocketAmmoType;
			}
		}
	}

	void GuiController::InsertObjectIntoList(int objectNumber)
	{
		Rings[(int)RingTypes::Inventory].CurrentObjectList[Rings[(int)RingTypes::Inventory].NumObjectsInList].InventoryItem = objectNumber;
		Rings[(int)RingTypes::Inventory].CurrentObjectList[Rings[(int)RingTypes::Inventory].NumObjectsInList].Orientation = EulerAngles::Zero;
		Rings[(int)RingTypes::Inventory].CurrentObjectList[Rings[(int)RingTypes::Inventory].NumObjectsInList].Bright = 32;
		Rings[(int)RingTypes::Inventory].NumObjectsInList++;
	}

	void GuiController::InsertObjectIntoList_v2(int objectNumber)
	{
		unsigned __int64 options = InventoryObjectTable[objectNumber].Options;

		if (options & (OPT_COMBINABLE | OPT_ALWAYS_COMBINE))
		{
			if (Rings[(int)RingTypes::Inventory].CurrentObjectList[Rings[(int)RingTypes::Inventory].CurrentObjectInList].InventoryItem != objectNumber)
			{
				Rings[(int)RingTypes::Ammo].CurrentObjectList[Rings[(int)RingTypes::Ammo].NumObjectsInList].InventoryItem = objectNumber;
				Rings[(int)RingTypes::Ammo].CurrentObjectList[Rings[(int)RingTypes::Ammo].NumObjectsInList].Orientation = EulerAngles::Zero;
				Rings[(int)RingTypes::Ammo].CurrentObjectList[Rings[(int)RingTypes::Ammo].NumObjectsInList++].Bright = 32;
			}
		}
	}

	void GuiController::ConstructObjectList(ItemInfo* item)
	{
		auto* lara = GetLaraInfo(item);

		Rings[(int)RingTypes::Inventory].NumObjectsInList = 0;

		for (int i = 0; i < INVENTORY_TABLE_SIZE; i++)
			Rings[(int)RingTypes::Inventory].CurrentObjectList[i].InventoryItem = NO_ITEM;

		Ammo.CurrentPistolsAmmoType = 0;
		Ammo.CurrentUziAmmoType = 0;
		Ammo.CurrentRevolverAmmoType = 0;
		Ammo.CurrentShotGunAmmoType = 0;
		Ammo.CurrentGrenadeGunAmmoType = 0;
		Ammo.CurrentCrossBowAmmoType = 0;

		if (g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() != LaraType::Young)
		{
			if (lara->Weapons[(int)LaraWeaponType::Pistol].Present)
				InsertObjectIntoList(INV_OBJECT_PISTOLS);
			else if (Ammo.AmountPistolsAmmo)
				InsertObjectIntoList(INV_OBJECT_PISTOLS_AMMO);

			if (lara->Weapons[(int)LaraWeaponType::Uzi].Present)
				InsertObjectIntoList(INV_OBJECT_UZIS);
			else if (Ammo.AmountUziAmmo)
				InsertObjectIntoList(INV_OBJECT_UZI_AMMO);

			if (lara->Weapons[(int)LaraWeaponType::Revolver].Present)
			{
				if (lara->Weapons[(int)LaraWeaponType::Revolver].HasLasersight)
					InsertObjectIntoList(INV_OBJECT_REVOLVER_LASER);
				else
					InsertObjectIntoList(INV_OBJECT_REVOLVER);
			}
			else if (Ammo.AmountRevolverAmmo)
				InsertObjectIntoList(INV_OBJECT_REVOLVER_AMMO);

			if (lara->Weapons[(int)LaraWeaponType::Shotgun].Present)
			{
				InsertObjectIntoList(INV_OBJECT_SHOTGUN);

				if (lara->Weapons[(int)LaraWeaponType::Shotgun].SelectedAmmo == WeaponAmmoType::Ammo2)
					Ammo.CurrentShotGunAmmoType = 1;
			}
			else
			{
				if (Ammo.AmountShotGunAmmo1)
					InsertObjectIntoList(INV_OBJECT_SHOTGUN_AMMO_1);

				if (Ammo.AmountShotGunAmmo2)
					InsertObjectIntoList(INV_OBJECT_SHOTGUN_AMMO_2);
			}

			if (lara->Weapons[(int)LaraWeaponType::HK].Present)
			{
				if (lara->Weapons[(int)LaraWeaponType::HK].HasLasersight)
					InsertObjectIntoList(INV_OBJECT_HK_LASERSIGHT);
				else
					InsertObjectIntoList(INV_OBJECT_HK);

				if (lara->Weapons[(int)LaraWeaponType::HK].WeaponMode == LaraWeaponTypeCarried::WTYPE_AMMO_2)
					Ammo.CurrentHKAmmoType = 1;

				if (lara->Weapons[(int)LaraWeaponType::HK].WeaponMode == LaraWeaponTypeCarried::WTYPE_AMMO_3)
					Ammo.CurrentHKAmmoType = 2;
			}
			else if (Ammo.AmountHKAmmo1)
				InsertObjectIntoList(INV_OBJECT_HK_AMMO);

			if (lara->Weapons[(int)LaraWeaponType::Crossbow].Present)
			{
				if (lara->Weapons[(int)LaraWeaponType::Crossbow].HasLasersight)
					InsertObjectIntoList(INV_OBJECT_CROSSBOW_LASER);
				else
					InsertObjectIntoList(INV_OBJECT_CROSSBOW);

				if (lara->Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo == WeaponAmmoType::Ammo2)
					Ammo.CurrentCrossBowAmmoType = 1;

				if (lara->Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo == WeaponAmmoType::Ammo3)
					Ammo.CurrentCrossBowAmmoType = 2;
			}
			else
			{
				if (Ammo.AmountCrossBowAmmo1)
					InsertObjectIntoList(INV_OBJECT_CROSSBOW_AMMO_1);

				if (Ammo.AmountCrossBowAmmo2)
					InsertObjectIntoList(INV_OBJECT_CROSSBOW_AMMO_2);

				if (Ammo.AmountCrossBowAmmo3)
					InsertObjectIntoList(INV_OBJECT_CROSSBOW_AMMO_3);
			}

			if (lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Present)
			{
				InsertObjectIntoList(INV_OBJECT_GRENADE_LAUNCHER);

				if (lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo == WeaponAmmoType::Ammo2)
					Ammo.CurrentGrenadeGunAmmoType = 1;

				if (lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo == WeaponAmmoType::Ammo3)
					Ammo.CurrentGrenadeGunAmmoType = 2;
			}
			else
			{
				if (Ammo.AmountGrenadeAmmo1)
					InsertObjectIntoList(INV_OBJECT_GRENADE_AMMO_1);

				if (Ammo.AmountGrenadeAmmo2)
					InsertObjectIntoList(INV_OBJECT_GRENADE_AMMO_2);

				if (Ammo.AmountGrenadeAmmo3)
					InsertObjectIntoList(INV_OBJECT_GRENADE_AMMO_3);
			}

			if (lara->Weapons[(int)LaraWeaponType::RocketLauncher].Present)
				InsertObjectIntoList(INV_OBJECT_ROCKET_LAUNCHER);
			else if (Ammo.AmountRocketsAmmo)
				InsertObjectIntoList(INV_OBJECT_ROCKET_AMMO);

			if (lara->Weapons[(int)LaraWeaponType::HarpoonGun].Present)
				InsertObjectIntoList(INV_OBJECT_HARPOON_GUN);
			else if (Ammo.AmountHarpoonAmmo)
				InsertObjectIntoList(INV_OBJECT_HARPOON_AMMO);

			if (lara->Inventory.HasLasersight)
				InsertObjectIntoList(INV_OBJECT_LASERSIGHT);

			if (lara->Inventory.HasSilencer)
				InsertObjectIntoList(INV_OBJECT_SILENCER);

			if (lara->Inventory.HasBinoculars)
				InsertObjectIntoList(INV_OBJECT_BINOCULARS);

			if (lara->Inventory.TotalFlares)
				InsertObjectIntoList(INV_OBJECT_FLARES);
		}

		InsertObjectIntoList(INV_OBJECT_TIMEX);//every level has the timex? what's a good way to check?!

		if (lara->Inventory.TotalSmallMedipacks)
			InsertObjectIntoList(INV_OBJECT_SMALL_MEDIPACK);

		if (lara->Inventory.TotalLargeMedipacks)
			InsertObjectIntoList(INV_OBJECT_LARGE_MEDIPACK);

		if (lara->Inventory.HasCrowbar)
			InsertObjectIntoList(INV_OBJECT_CROWBAR);

		if (lara->Inventory.BeetleComponents)
		{
			if (lara->Inventory.BeetleComponents & BEETLECOMP_FLAG_BEETLE)
				InsertObjectIntoList(INV_OBJECT_BEETLE);

			if (lara->Inventory.BeetleComponents & BEETLECOMP_FLAG_COMBO_1)
				InsertObjectIntoList(INV_OBJECT_BEETLE_PART1);

			if (lara->Inventory.BeetleComponents & BEETLECOMP_FLAG_COMBO_2)
				InsertObjectIntoList(INV_OBJECT_BEETLE_PART2);
		}

		if (lara->Inventory.SmallWaterskin)
			InsertObjectIntoList((lara->Inventory.SmallWaterskin - 1) + INV_OBJECT_SMALL_WATERSKIN_EMPTY);

		if (lara->Inventory.BigWaterskin)
			InsertObjectIntoList((lara->Inventory.BigWaterskin - 1) + INV_OBJECT_BIG_WATERSKIN_EMPTY);

		for (int i = 0; i < NUM_PUZZLES; i++)
		{
			if (lara->Inventory.Puzzles[i])
				InsertObjectIntoList(INV_OBJECT_PUZZLE1 + i);
		}

		for (int i = 0; i < NUM_PUZZLE_PIECES; i++)
		{
			if (lara->Inventory.PuzzlesCombo[i])
				InsertObjectIntoList(INV_OBJECT_PUZZLE1_COMBO1 + i);
		}

		for (int i = 0; i < NUM_KEYS; i++)
		{
			if (lara->Inventory.Keys[i])
				InsertObjectIntoList(INV_OBJECT_KEY1 + i);
		}

		for (int i = 0; i < NUM_KEY_PIECES; i++)
		{
			if (lara->Inventory.KeysCombo[i])
				InsertObjectIntoList(INV_OBJECT_KEY1_COMBO1 + i);
		}

		for (int i = 0; i < NUM_PICKUPS; i++)
		{
			if (lara->Inventory.Pickups[i])
				InsertObjectIntoList(INV_OBJECT_PICKUP1 + i);
		}

		for (int i = 0; i < NUM_PICKUPS_PIECES; i++)
		{
			if (lara->Inventory.PickupsCombo[i])
				InsertObjectIntoList(INV_OBJECT_PICKUP1_COMBO1 + i);
		}

		for (int i = 0; i < NUM_EXAMINES; i++)
		{
			if (lara->Inventory.Examines[i])
				InsertObjectIntoList(INV_OBJECT_EXAMINE1 + i);
		}

		for (int i = 0; i < NUM_EXAMINES_PIECES; i++)
		{
			if (lara->Inventory.ExaminesCombo[i])
				InsertObjectIntoList(INV_OBJECT_EXAMINE1_COMBO1 + i);
		}

		if (lara->Inventory.Diary.Present)
			InsertObjectIntoList(INV_OBJECT_DIARY);

		if (g_GameFlow->IsLoadSaveEnabled())
		{
			InsertObjectIntoList(INV_OBJECT_LOAD_FLOPPY);
			InsertObjectIntoList(INV_OBJECT_SAVE_FLOPPY);
		}

		Rings[(int)RingTypes::Inventory].ObjectListMovement = 0;
		Rings[(int)RingTypes::Inventory].CurrentObjectInList = 0;
		Rings[(int)RingTypes::Inventory].RingActive = true;
		Rings[(int)RingTypes::Ammo].ObjectListMovement = 0;
		Rings[(int)RingTypes::Ammo].CurrentObjectInList = 0;
		Rings[(int)RingTypes::Ammo].RingActive = false;
		HandleObjectChangeover((int)RingTypes::Inventory);
		AmmoActive = 0;
	}

	void GuiController::ConstructCombineObjectList(ItemInfo* item)
	{
		auto* lara = GetLaraInfo(item);

		Rings[(int)RingTypes::Ammo].NumObjectsInList = 0;

		for (int i = 0; i < INVENTORY_TABLE_SIZE; i++)
			Rings[(int)RingTypes::Ammo].CurrentObjectList[i].InventoryItem = NO_ITEM;

		if (!(g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young))
		{
			if (lara->Weapons[(int)LaraWeaponType::Revolver].Present)
			{
				if (lara->Weapons[(int)LaraWeaponType::Revolver].HasLasersight)
					InsertObjectIntoList_v2(INV_OBJECT_REVOLVER_LASER);
				else
					InsertObjectIntoList_v2(INV_OBJECT_REVOLVER);
			}

			if (lara->Weapons[(int)LaraWeaponType::HK].Present)
			{
				if (lara->Weapons[(int)LaraWeaponType::HK].HasLasersight)
					InsertObjectIntoList_v2(INV_OBJECT_HK_LASERSIGHT);
				else
					InsertObjectIntoList_v2(INV_OBJECT_HK);
			}

			if (lara->Weapons[(int)LaraWeaponType::Crossbow].Present)
			{
				if (lara->Weapons[(int)LaraWeaponType::Crossbow].HasLasersight)
					InsertObjectIntoList_v2(INV_OBJECT_CROSSBOW_LASER);
				else
					InsertObjectIntoList_v2(INV_OBJECT_CROSSBOW);
			}

			if (lara->Inventory.HasLasersight)
				InsertObjectIntoList_v2(INV_OBJECT_LASERSIGHT);

			if (lara->Inventory.HasSilencer)
				InsertObjectIntoList_v2(INV_OBJECT_SILENCER);
		}

		if (lara->Inventory.BeetleComponents)
		{
			if (lara->Inventory.BeetleComponents & 2)
				InsertObjectIntoList_v2(INV_OBJECT_BEETLE_PART1);

			if (lara->Inventory.BeetleComponents & 4)
				InsertObjectIntoList_v2(INV_OBJECT_BEETLE_PART2);
		}

		if (lara->Inventory.SmallWaterskin)
			InsertObjectIntoList_v2(lara->Inventory.SmallWaterskin - 1 + INV_OBJECT_SMALL_WATERSKIN_EMPTY);

		if (lara->Inventory.BigWaterskin)
			InsertObjectIntoList_v2(lara->Inventory.BigWaterskin - 1 + INV_OBJECT_BIG_WATERSKIN_EMPTY);

		for (int i = 0; i < NUM_PUZZLE_PIECES; i++)
		{
			if (lara->Inventory.PuzzlesCombo[i])
				InsertObjectIntoList_v2(INV_OBJECT_PUZZLE1_COMBO1 + i);
		}

		for (int i = 0; i < NUM_KEY_PIECES; i++)
		{
			if (lara->Inventory.KeysCombo[i])
				InsertObjectIntoList_v2(INV_OBJECT_KEY1_COMBO1 + i);
		}

		for (int i = 0; i < NUM_PICKUPS_PIECES; i++)
		{
			if (lara->Inventory.PickupsCombo[i])
				InsertObjectIntoList_v2(INV_OBJECT_PICKUP1_COMBO1 + i);
		}

		for (int i = 0; i < NUM_EXAMINES_PIECES; i++)
		{
			if (lara->Inventory.ExaminesCombo[i])
				InsertObjectIntoList_v2(INV_OBJECT_EXAMINE1_COMBO1 + i);
		}

		Rings[(int)RingTypes::Ammo].ObjectListMovement = 0;
		Rings[(int)RingTypes::Ammo].CurrentObjectInList = 0;
		Rings[(int)RingTypes::Ammo].RingActive = false;
	}

	void GuiController::Initialize()
	{
		g_Gui.SetMenuToDisplay(Menu::Title);
		g_Gui.SetSelectedOption(0);
		g_Gui.SetLastInventoryItem(NO_ITEM);
	}

	void GuiController::InitializeInventory(ItemInfo* item)
	{
		auto* lara = GetLaraInfo(item);

		AlterFOV(ANGLE(DEFAULT_FOV), false);
		lara->Inventory.IsBusy = false;
		InventoryItemChosen = NO_ITEM;
		UseItem = false;

		if (lara->Weapons[(int)LaraWeaponType::Shotgun].Ammo[0].HasInfinite())
		{
			Ammo.AmountShotGunAmmo1 = -1;
		}
		else
		{
			Ammo.AmountShotGunAmmo1 = lara->Weapons[(int)LaraWeaponType::Shotgun].Ammo[0].GetCount() / 6;
		}

		if (lara->Weapons[(int)LaraWeaponType::Shotgun].Ammo[1].HasInfinite())
		{
			Ammo.AmountShotGunAmmo2 = -1;
		}
		else
		{
			Ammo.AmountShotGunAmmo2 = lara->Weapons[(int)LaraWeaponType::Shotgun].Ammo[1].GetCount() / 6;
		}
		
		Ammo.AmountShotGunAmmo1 = lara->Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo1].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
		Ammo.AmountShotGunAmmo2 = lara->Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo2].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo2].GetCount();
		Ammo.AmountHKAmmo1 = lara->Weapons[(int)LaraWeaponType::HK].Ammo[(int)WeaponAmmoType::Ammo1].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::HK].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
		Ammo.AmountCrossBowAmmo1 = lara->Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo1].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
		Ammo.AmountCrossBowAmmo2 = lara->Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo2].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo2].GetCount();
		Ammo.AmountCrossBowAmmo3 = lara->Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo3].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo3].GetCount();
		Ammo.AmountUziAmmo = lara->Weapons[(int)LaraWeaponType::Uzi].Ammo[(int)WeaponAmmoType::Ammo1].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::Uzi].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
		Ammo.AmountRevolverAmmo = lara->Weapons[(int)LaraWeaponType::Revolver].Ammo[(int)WeaponAmmoType::Ammo1].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::Revolver].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
		Ammo.AmountPistolsAmmo = lara->Weapons[(int)LaraWeaponType::Pistol].Ammo[(int)WeaponAmmoType::Ammo1].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::Pistol].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
		Ammo.AmountRocketsAmmo = lara->Weapons[(int)LaraWeaponType::RocketLauncher].Ammo[(int)WeaponAmmoType::Ammo1].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::RocketLauncher].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
		Ammo.AmountHarpoonAmmo = lara->Weapons[(int)LaraWeaponType::HarpoonGun].Ammo[(int)WeaponAmmoType::Ammo1].HasInfinite()? -1 : lara->Weapons[(int)LaraWeaponType::HarpoonGun].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
		Ammo.AmountGrenadeAmmo1 = lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo1].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
		Ammo.AmountGrenadeAmmo2 = lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo2].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo2].GetCount();
		Ammo.AmountGrenadeAmmo3 = lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo3].HasInfinite() ? -1 : lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo3].GetCount();
		ConstructObjectList(item);

		if (EnterInventory == NO_ITEM)
		{
			if (LastInvItem != NO_ITEM)
			{
				if (IsItemInInventory(LastInvItem))
				{
					SetupObjectListStartPosition(LastInvItem);
				}
				else
				{
					if (LastInvItem >= INV_OBJECT_SMALL_WATERSKIN_EMPTY && LastInvItem <= INV_OBJECT_SMALL_WATERSKIN_3L)
					{
						for (int i = INV_OBJECT_SMALL_WATERSKIN_EMPTY; i <= INV_OBJECT_SMALL_WATERSKIN_3L; i++)
						{
							if (IsItemInInventory(i))
							{
								SetupObjectListStartPosition(i);
								break;
							}
						}
					}
					else if (LastInvItem >= INV_OBJECT_BIG_WATERSKIN_EMPTY && LastInvItem <= INV_OBJECT_BIG_WATERSKIN_5L)
					{
						for (int i = INV_OBJECT_BIG_WATERSKIN_EMPTY; i <= INV_OBJECT_BIG_WATERSKIN_5L; i++)
						{
							if (IsItemInInventory(i))
							{
								SetupObjectListStartPosition(i);
								break;
							}
						}
					}
					else
					{
						LastInvItem = NO_ITEM;
					}
				}
			}
		}
		else
		{
			if (IsObjectInInventory(EnterInventory))
				SetupObjectListStartPosition2(EnterInventory);
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
			if (InventoryObjectTable[Rings[(int)RingTypes::Inventory].CurrentObjectList[i].InventoryItem].ObjectNumber == newObjectNumber)
				Rings[(int)RingTypes::Inventory].CurrentObjectInList = i;
		}
	}

	int GuiController::ConvertObjectToInventoryItem(int objectNumber)
	{
		for (int i = 0; i < INVENTORY_TABLE_SIZE; i++)
		{
			if (InventoryObjectTable[i].ObjectNumber == objectNumber)
				return i;
		}

		return -1;
	}

	int GuiController::ConvertInventoryItemToObject(int objectNumber)
	{
		return InventoryObjectTable[objectNumber].ObjectNumber;
	}

	void GuiController::FadeAmmoSelector()
	{
		if (Rings[(int)RingTypes::Inventory].RingActive)
		{
			AmmoSelectorFadeVal = 0;
		}
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

	void GuiController::UseCurrentItem(ItemInfo* item)
	{
		const std::vector<int> CrouchStates =
		{
			LS_CROUCH_IDLE,
			LS_CROUCH_TURN_LEFT,
			LS_CROUCH_TURN_RIGHT,
			LS_CROUCH_TURN_180
		};
		const std::vector<int> CrawlStates =
		{
			LS_CRAWL_IDLE,
			LS_CRAWL_FORWARD,
			LS_CRAWL_BACK,
			LS_CRAWL_TURN_LEFT,
			LS_CRAWL_TURN_RIGHT,
			LS_CRAWL_TURN_180,
			LS_CRAWL_TO_HANG
		};

		auto* lara = GetLaraInfo(item);

		int prevOpticRange = lara->Control.Look.OpticRange;
		short inventoryObject = Rings[(int)RingTypes::Inventory].CurrentObjectList[Rings[(int)RingTypes::Inventory].CurrentObjectInList].InventoryItem;
		short gameObject = InventoryObjectTable[inventoryObject].ObjectNumber;

		item->MeshBits = ALL_JOINT_BITS;
		lara->Control.Look.OpticRange = 0;
		lara->Inventory.OldBusy = false;

		if (lara->Control.WaterStatus == WaterStatus::Dry ||
			lara->Control.WaterStatus == WaterStatus::Wade)
		{
			if (gameObject == ID_PISTOLS_ITEM)
			{
				lara->Control.Weapon.RequestGunType = LaraWeaponType::Pistol;

				if (lara->Control.HandStatus != HandStatus::Free)
					return;

				if (lara->Control.Weapon.GunType == LaraWeaponType::Pistol)
					lara->Control.HandStatus = HandStatus::WeaponDraw;

				return;
			}

			if (gameObject == ID_UZI_ITEM)
			{
				lara->Control.Weapon.RequestGunType = LaraWeaponType::Uzi;

				if (lara->Control.HandStatus != HandStatus::Free)
					return;

				if (lara->Control.Weapon.GunType == LaraWeaponType::Uzi)
					lara->Control.HandStatus = HandStatus::WeaponDraw;

				return;
			}
		}

		if (gameObject != ID_SHOTGUN_ITEM &&
			gameObject != ID_REVOLVER_ITEM &&
			gameObject != ID_HK_ITEM &&
			gameObject != ID_CROSSBOW_ITEM &&
			gameObject != ID_GRENADE_GUN_ITEM &&
			gameObject != ID_ROCKET_LAUNCHER_ITEM &&
			gameObject != ID_HARPOON_ITEM)
		{
			if (gameObject == ID_FLARE_INV_ITEM)
			{
				if (lara->Control.HandStatus == HandStatus::Free)
				{
					if (!TestState(item->Animation.ActiveState, CrawlStates))
					{
						if (lara->Control.Weapon.GunType != LaraWeaponType::Flare)
						{
							// HACK.
							ClearAllActions();
							ActionMap[(int)In::Flare].Update(1.0f);

							HandleWeapon(*item);
							ClearAllActions();
						}

						return;
					}
				}

				SayNo();
				return;
			}

			switch (inventoryObject)
			{
			case INV_OBJECT_BINOCULARS:
				if (((item->Animation.ActiveState == LS_IDLE && item->Animation.AnimNumber == LA_STAND_IDLE) ||
						(lara->Control.IsLow && !IsHeld(In::Crouch))) &&
					!UseSpotCam && !TrackCameraInit)
				{
					lara->Control.Look.OpticRange = 128;
					lara->Control.Look.IsUsingBinoculars = true;
					lara->Inventory.OldBusy = true;

					// TODO: To prevent Lara from crouching or performing other actions, the inherent state of
					// LA_BINOCULARS_IDLE must be changed to LS_IDLE. @Sezz 2022.05.19
					//SetAnimation(item, LA_BINOCULARS_IDLE);

					if (lara->Control.HandStatus != HandStatus::Free)
						lara->Control.HandStatus = HandStatus::WeaponUndraw;
				}

				if (prevOpticRange)
				{
					lara->Control.Look.OpticRange = prevOpticRange;
				}
				else
				{
					BinocularOldCamera = Camera.oldType;
				}

				return;

			case INV_OBJECT_SMALL_MEDIPACK:

				if ((item->HitPoints <= 0 || item->HitPoints >= LARA_HEALTH_MAX) &&
					lara->Status.Poison == 0)
				{
					SayNo();
					return;
				}

				if (lara->Inventory.TotalSmallMedipacks != 0)
				{
					if (lara->Inventory.TotalSmallMedipacks != -1)
						lara->Inventory.TotalSmallMedipacks--;

					lara->Status.Poison = 0;
					item->HitPoints += LARA_HEALTH_MAX / 2;

					if (item->HitPoints > LARA_HEALTH_MAX)
						item->HitPoints = LARA_HEALTH_MAX;

					SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always);
					Statistics.Game.HealthUsed++;
				}
				else
					SayNo();

				return;

			case INV_OBJECT_LARGE_MEDIPACK:

				if ((item->HitPoints <= 0 || item->HitPoints >= LARA_HEALTH_MAX) &&
					lara->Status.Poison == 0)
				{
					SayNo();
					return;
				}

				if (lara->Inventory.TotalLargeMedipacks != 0)
				{
					if (lara->Inventory.TotalLargeMedipacks != -1)
						lara->Inventory.TotalLargeMedipacks--;

					lara->Status.Poison = 0;
					item->HitPoints = LARA_HEALTH_MAX;

					SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always);
					Statistics.Game.HealthUsed++;
				}
				else
					SayNo();

				return;

			default:
				InventoryItemChosen = gameObject;
				return;
			}

			return;
		}

		if (lara->Control.HandStatus == HandStatus::Busy)
		{
			SayNo();
			return;
		}

		if (TestState(item->Animation.ActiveState, CrouchStates) ||
			TestState(item->Animation.ActiveState, CrawlStates))
		{
			SayNo();
			return;
		}

		if (gameObject == ID_SHOTGUN_ITEM)
		{
			lara->Control.Weapon.RequestGunType = LaraWeaponType::Shotgun;

			if (lara->Control.HandStatus != HandStatus::Free)
				return;

			if (lara->Control.Weapon.GunType == LaraWeaponType::Shotgun)
				lara->Control.HandStatus = HandStatus::WeaponDraw;

			return;
		}

		if (gameObject == ID_REVOLVER_ITEM)
		{
			lara->Control.Weapon.RequestGunType = LaraWeaponType::Revolver;

			if (lara->Control.HandStatus != HandStatus::Free)
				return;

			if (lara->Control.Weapon.GunType == LaraWeaponType::Revolver)
				lara->Control.HandStatus = HandStatus::WeaponDraw;

			return;
		}
		else if (gameObject == ID_HK_ITEM)
		{
			lara->Control.Weapon.RequestGunType = LaraWeaponType::HK;

			if (lara->Control.HandStatus != HandStatus::Free)
				return;

			if (lara->Control.Weapon.GunType == LaraWeaponType::HK)
				lara->Control.HandStatus = HandStatus::WeaponDraw;

			return;
		}
		else if (gameObject == ID_CROSSBOW_ITEM)
		{
			lara->Control.Weapon.RequestGunType = LaraWeaponType::Crossbow;

			if (lara->Control.HandStatus != HandStatus::Free)
				return;

			if (lara->Control.Weapon.GunType == LaraWeaponType::Crossbow)
				lara->Control.HandStatus = HandStatus::WeaponDraw;

			return;
		}
		else if (gameObject == ID_GRENADE_GUN_ITEM)
		{
			lara->Control.Weapon.RequestGunType = LaraWeaponType::GrenadeLauncher;

			if (lara->Control.HandStatus != HandStatus::Free)
				return;

			if (lara->Control.Weapon.GunType == LaraWeaponType::GrenadeLauncher)
				lara->Control.HandStatus = HandStatus::WeaponDraw;

			return;
		}
		else if (gameObject == ID_HARPOON_ITEM)
		{
			lara->Control.Weapon.RequestGunType = LaraWeaponType::HarpoonGun;

			if (lara->Control.HandStatus != HandStatus::Free)
				return;

			if (lara->Control.Weapon.GunType == LaraWeaponType::HarpoonGun)
				lara->Control.HandStatus = HandStatus::WeaponDraw;

			return;
		}
		else if (gameObject == ID_ROCKET_LAUNCHER_ITEM)
		{
			lara->Control.Weapon.RequestGunType = LaraWeaponType::RocketLauncher;

			if (lara->Control.HandStatus != HandStatus::Free)
				return;

			if (lara->Control.Weapon.GunType == LaraWeaponType::RocketLauncher)
				lara->Control.HandStatus = HandStatus::WeaponDraw;

			return;
		}
	}

	void GuiController::DoInventory(ItemInfo* item)
	{
		auto* lara = GetLaraInfo(item);
		auto& invRing = Rings[(int)RingTypes::Inventory];
		auto& ammoRing = Rings[(int)RingTypes::Ammo];

		if (ammoRing.RingActive)
		{
			auto optionString = g_GameFlow->GetString(OptionStrings[5].c_str());
			g_Renderer.AddString(PHD_CENTER_X, PHD_CENTER_Y, optionString, PRINTSTRING_COLOR_WHITE, PRINTSTRING_BLINK | PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);

			if (invRing.ObjectListMovement)
				return;

			if (ammoRing.ObjectListMovement)
				return;

			if (GuiIsSelected(false))
			{
				short invItem = invRing.CurrentObjectList[invRing.CurrentObjectInList].InventoryItem;
				short ammoItem = ammoRing.CurrentObjectList[ammoRing.CurrentObjectInList].InventoryItem;

				if (DoObjectsCombine(invItem, ammoItem))
				{
					CombineRingFadeDir = 2;
					CombineTypeFlag = 1;
					CombineObject1 = invItem;
					CombineObject2 = ammoItem;
					SoundEffect(SFX_TR4_MENU_COMBINE, nullptr, SoundEnvironment::Always);
				}
				else if (ammoItem >= INV_OBJECT_SMALL_WATERSKIN_EMPTY &&
						 ammoItem <= INV_OBJECT_SMALL_WATERSKIN_3L &&
						 invItem >= INV_OBJECT_BIG_WATERSKIN_EMPTY &&
						 invItem <= INV_OBJECT_BIG_WATERSKIN_5L)
				{
					if (PerformWaterskinCombine(item, true))
					{
						CombineTypeFlag = 2;
						CombineRingFadeDir = 2;
						SoundEffect(SFX_TR4_MENU_COMBINE, nullptr, SoundEnvironment::Always);
						return;
					}

					SayNo();
					CombineRingFadeDir = 2;
				}
				else if (invItem >= INV_OBJECT_SMALL_WATERSKIN_EMPTY &&
						 invItem <= INV_OBJECT_SMALL_WATERSKIN_3L &&
						 ammoItem >= INV_OBJECT_BIG_WATERSKIN_EMPTY &&
						 ammoItem <= INV_OBJECT_BIG_WATERSKIN_5L)
				{
					if (PerformWaterskinCombine(item, false))
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

			if (GuiIsDeselected())
			{
				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
				CombineRingFadeDir = 2;
			}

			return;
		}
		else
		{
			int num = invRing.CurrentObjectList[invRing.CurrentObjectInList].InventoryItem;

			for (int i = 0; i < 3; i++)
			{
				CurrentOptions[i].Type = MenuType::None;
				CurrentOptions[i].Text = {};
			}

			int n = 0;
			unsigned long options;
			if (!AmmoActive)
			{
				options = InventoryObjectTable[invRing.CurrentObjectList[invRing.CurrentObjectInList].InventoryItem].Options;

				if (options & OPT_LOAD)
				{
					CurrentOptions[0].Type = MenuType::Load;
					CurrentOptions[0].Text = g_GameFlow->GetString(OptionStrings[6].c_str());
					n = 1;
				}

				if (options & OPT_SAVE)
				{
					CurrentOptions[n].Type = MenuType::Save;
					CurrentOptions[n].Text = g_GameFlow->GetString(OptionStrings[7].c_str());
					n++;
				}

				if (options & OPT_EXAMINABLE)
				{
					CurrentOptions[n].Type = MenuType::Examine;
					CurrentOptions[n].Text = g_GameFlow->GetString(OptionStrings[8].c_str());
					n++;
				}

				if (options & OPT_STATS)
				{
					CurrentOptions[n].Type = MenuType::Statistics;
					CurrentOptions[n].Text = g_GameFlow->GetString(OptionStrings[9].c_str());
					n++;
				}

				if (options & OPT_USE)
				{
					CurrentOptions[n].Type = MenuType::Use;
					CurrentOptions[n].Text = g_GameFlow->GetString(OptionStrings[0].c_str());
					n++;
				}

				if (options & OPT_EQUIP)
				{
					CurrentOptions[n].Type = MenuType::Equip;
					CurrentOptions[n].Text = g_GameFlow->GetString(OptionStrings[4].c_str());
					n++;
				}

				if (options & (OPT_CHOOSE_AMMO_SHOTGUN | OPT_CHOOSE_AMMO_CROSSBOW | OPT_CHOOSE_AMMO_GRENADEGUN | OPT_CHOOSE_AMMO_HK))
				{
					CurrentOptions[n].Type = MenuType::ChooseAmmo;
					CurrentOptions[n].Text = g_GameFlow->GetString(OptionStrings[1].c_str());
					n++;
				}

				if (options & OPT_COMBINABLE)
				{
					if (IsItemCurrentlyCombinable(num))
					{
						CurrentOptions[n].Type = MenuType::Combine;
						CurrentOptions[n].Text = g_GameFlow->GetString(OptionStrings[2].c_str());
						n++;
					}
				}

				if (options & OPT_ALWAYS_COMBINE)
				{
					CurrentOptions[n].Type = MenuType::Combine;
					CurrentOptions[n].Text = g_GameFlow->GetString(OptionStrings[2].c_str());
					n++;
				}

				if (options & OPT_SEPERABLE)
				{
					CurrentOptions[n].Type = MenuType::Seperate;
					CurrentOptions[n].Text = g_GameFlow->GetString(OptionStrings[3].c_str());
					n++;
				}

				if (options & OPT_DIARY)
				{
					CurrentOptions[n].Type = MenuType::Diary;
					CurrentOptions[n].Text = g_GameFlow->GetString(OptionStrings[11].c_str());
					n++;
				}
			}
			else
			{
				CurrentOptions[0].Type = MenuType::Ammo1;
				CurrentOptions[0].Text = g_GameFlow->GetString(InventoryObjectTable[AmmoObjectList[0].InventoryItem].ObjectName);
				CurrentOptions[1].Type = MenuType::Ammo2;
				CurrentOptions[1].Text = g_GameFlow->GetString(InventoryObjectTable[AmmoObjectList[1].InventoryItem].ObjectName);
				n = 2;

				options = InventoryObjectTable[invRing.CurrentObjectList[invRing.CurrentObjectInList].InventoryItem].Options;

				if (options & (OPT_CHOOSE_AMMO_CROSSBOW | OPT_CHOOSE_AMMO_GRENADEGUN | OPT_CHOOSE_AMMO_HK))
				{
					n = 3;
					CurrentOptions[2].Type = MenuType::Ammo3;
					CurrentOptions[2].Text = g_GameFlow->GetString(InventoryObjectTable[AmmoObjectList[2].InventoryItem].ObjectName);
				}

				CurrentSelectedOption = *CurrentAmmoType;
			}

			int yPos = 310 - LINE_HEIGHT;

			if (n == 1)
			{
				yPos += LINE_HEIGHT;
			}
			else if (n == 2)
			{
				yPos += LINE_HEIGHT / 2;
			}

			if (n > 0)
			{
				for (int i = 0; i < n; i++)
				{
					auto optionString = CurrentOptions[i].Text;

					if (i == CurrentSelectedOption)
					{
						g_Renderer.AddString(PHD_CENTER_X, yPos, optionString.c_str(), PRINTSTRING_COLOR_WHITE, PRINTSTRING_BLINK | PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
						yPos += LINE_HEIGHT;
					}
					else
					{
						g_Renderer.AddString(PHD_CENTER_X, yPos, optionString.c_str(), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
						yPos += LINE_HEIGHT;
					}
				}
			}

			if (MenuActive &&
				!invRing.ObjectListMovement &&
				!ammoRing.ObjectListMovement)
			{
				if (AmmoActive)
				{
					if (GuiIsPulsed(In::Forward))
					{
						if (CurrentSelectedOption <= 0)
							CurrentSelectedOption = n - 1;
						else
							CurrentSelectedOption--;

						SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
					}

					if (GuiIsPulsed(In::Back))
					{
						if (CurrentSelectedOption >= (n - 1))
							CurrentSelectedOption = 0;
						else
							CurrentSelectedOption++;

						SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
					}

					*CurrentAmmoType = CurrentSelectedOption;
				}
				else
				{
					if (GuiIsPulsed(In::Forward))
					{
						if (CurrentSelectedOption <= 0)
							CurrentSelectedOption = n - 1;
						else
							CurrentSelectedOption--;

						SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
					}
					else if (GuiIsPulsed(In::Back))
					{
						if (CurrentSelectedOption >= (n - 1))
							CurrentSelectedOption = 0;
						else
							CurrentSelectedOption++;

						SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
					}
				}

				if (GuiIsSelected(false))
				{
					if (CurrentOptions[CurrentSelectedOption].Type != MenuType::Equip &&
						CurrentOptions[CurrentSelectedOption].Type != MenuType::Use)
					{
						SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
					}

					switch (CurrentOptions[CurrentSelectedOption].Type)
					{
					case MenuType::ChooseAmmo:
						invRing.RingActive = false;
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
						SaveGame::LoadSavegameInfos();
						SetInventoryMode(InventoryMode::Load);
						break;

					case MenuType::Save:
						SaveGame::LoadSavegameInfos();
						SetInventoryMode(InventoryMode::Save);
						break;

					case MenuType::Examine:
						SetInventoryMode(InventoryMode::Examine);
						break;

					case MenuType::Statistics:
						SetInventoryMode(InventoryMode::Statistics);
						break;

					case MenuType::Ammo1:
					case MenuType::Ammo2:
					case MenuType::Ammo3:
						AmmoActive = 0;
						invRing.RingActive = true;
						CurrentSelectedOption = 0;
						break;

					case MenuType::Combine:
						ConstructCombineObjectList(item);
						invRing.RingActive = false;
						ammoRing.RingActive = true;
						AmmoSelectorFlag = 0;
						MenuActive = false;
						CombineRingFadeDir = 1;
						break;

					case MenuType::Seperate:
						SeperateTypeFlag = 1;
						NormalRingFadeDir = 2;
						break;

					case MenuType::Equip:
					case MenuType::Use:
						MenuActive = false;
						UseItem = true;
						break;

					case MenuType::Diary:
						SetInventoryMode(InventoryMode::Diary);
						lara->Inventory.Diary.CurrentPage = 1;
						break;
					}
				}

				if (GuiIsDeselected() && AmmoActive)
				{
					SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
					AmmoActive = 0;
					invRing.RingActive = true;
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
	void GuiController::UpdateWeaponStatus(ItemInfo* item)
	{
		auto* lara = GetLaraInfo(item);

		if (lara->Weapons[(int)LaraWeaponType::Shotgun].Present)
		{
			if (Ammo.CurrentShotGunAmmoType)
				lara->Weapons[(int)LaraWeaponType::Shotgun].SelectedAmmo = WeaponAmmoType::Ammo2;
			else
				lara->Weapons[(int)LaraWeaponType::Shotgun].SelectedAmmo = WeaponAmmoType::Ammo1;
		}

		if (lara->Weapons[(int)LaraWeaponType::Crossbow].Present)
		{
			lara->Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo = WeaponAmmoType::Ammo1;

			if (Ammo.CurrentCrossBowAmmoType == 1)
				lara->Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo = WeaponAmmoType::Ammo2;
			else if (Ammo.CurrentCrossBowAmmoType == 2)
				lara->Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo = WeaponAmmoType::Ammo3;
		}

		if (lara->Weapons[(int)LaraWeaponType::HK].Present)
		{
			lara->Weapons[(int)LaraWeaponType::HK].WeaponMode = LaraWeaponTypeCarried::WTYPE_AMMO_1;
			lara->Weapons[(int)LaraWeaponType::HK].SelectedAmmo = WeaponAmmoType::Ammo1;

			if (Ammo.CurrentHKAmmoType == 1)
			{
				lara->Weapons[(int)LaraWeaponType::HK].WeaponMode = LaraWeaponTypeCarried::WTYPE_AMMO_2;
				lara->Weapons[(int)LaraWeaponType::HK].SelectedAmmo = WeaponAmmoType::Ammo1;
			}
			else if (Ammo.CurrentHKAmmoType == 2)
			{
				lara->Weapons[(int)LaraWeaponType::HK].WeaponMode = LaraWeaponTypeCarried::WTYPE_AMMO_3;
				lara->Weapons[(int)LaraWeaponType::HK].SelectedAmmo = WeaponAmmoType::Ammo1;
			}
		}

		if (lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Present)
		{
			lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo = WeaponAmmoType::Ammo1;

			if (Ammo.CurrentGrenadeGunAmmoType == 1)
				lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo = WeaponAmmoType::Ammo2;
			else if (Ammo.CurrentGrenadeGunAmmoType == 2)
				lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo = WeaponAmmoType::Ammo3;
		}
	}

	void GuiController::SpinBack(EulerAngles& orient)
	{
		orient.Lerp(EulerAngles::Zero, 1.0f / 8);
	}

	void GuiController::DrawAmmoSelector()
	{
		if (!AmmoSelectorFlag)
			return;
	
		int xPos = (2 * PHD_CENTER_X - OBJLIST_SPACING) / 2;
		if (NumAmmoSlots == 2)
			xPos -= OBJLIST_SPACING / 2;
		else if (NumAmmoSlots == 3)
			xPos -= OBJLIST_SPACING;

		if (NumAmmoSlots > 0)
		{
			for (int n = 0; n < NumAmmoSlots; n++)
			{
				auto* invObject = &InventoryObjectTable[AmmoObjectList[n].InventoryItem];

				if (n == *CurrentAmmoType)
				{
					if (invObject->RotFlags & INV_ROT_X)
						AmmoObjectList[n].Orientation.x += ANGLE(5.0f);

					if (invObject->RotFlags & INV_ROT_Y)
						AmmoObjectList[n].Orientation.y += ANGLE(5.0f);

					if (invObject->RotFlags & INV_ROT_Z)
						AmmoObjectList[n].Orientation.z += ANGLE(5.0f);
				}
				else
					SpinBack(AmmoObjectList[n].Orientation);

				int x = PHD_CENTER_X - 300 + xPos;
				int y = 480;
				short objectNumber = ConvertInventoryItemToObject(AmmoObjectList[n].InventoryItem);
				float scaler = InventoryObjectTable[AmmoObjectList[n].InventoryItem].Scale1;

				if (n == *CurrentAmmoType)
				{
					char invTextBuffer[256];

					if (AmmoObjectList[n].Amount == -1)
					{
						sprintf(&invTextBuffer[0], g_GameFlow->GetString(STRING_UNLIMITED), g_GameFlow->GetString(InventoryObjectTable[AmmoObjectList[n].InventoryItem].ObjectName));
					}
					else
					{
						sprintf(&invTextBuffer[0], "%d x %s", AmmoObjectList[n].Amount, g_GameFlow->GetString(InventoryObjectTable[AmmoObjectList[n].InventoryItem].ObjectName));
					}

					// CHECK: AmmoSelectorFadeVal is never true and therefore the string is never printed.
					//if (AmmoSelectorFadeVal)
						g_Renderer.AddString(PHD_CENTER_X, 380, &invTextBuffer[0], PRINTSTRING_COLOR_YELLOW, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
				
					if (n == *CurrentAmmoType)
						g_Renderer.DrawObjectIn2DSpace(objectNumber, Vector2(x, y), AmmoObjectList[n].Orientation, scaler);
					else
						g_Renderer.DrawObjectIn2DSpace(objectNumber, Vector2(x, y), AmmoObjectList[n].Orientation, scaler);
				}
				else
				{
					g_Renderer.DrawObjectIn2DSpace(objectNumber, Vector2(x, y), AmmoObjectList[n].Orientation, scaler);
				}

				xPos += OBJLIST_SPACING;
			}
		}
	}

	void GuiController::DrawCurrentObjectList(ItemInfo* item, RingTypes ringType)
	{
		const auto& player = GetLaraInfo(*item);
		auto& ring = Rings[(int)ringType];

		if (ring.CurrentObjectList <= 0)
			return;

		if (ringType == RingTypes::Ammo)
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
					{
						NormalRingFadeDir = 2;
					}
					else
					{
						Rings[(int)RingTypes::Inventory].RingActive = true;
						MenuActive = true;
						Rings[(int)RingTypes::Ammo].RingActive = false;
						HandleObjectChangeover((int)RingTypes::Inventory);
					}

					Rings[(int)RingTypes::Ammo].RingActive = false;
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
				Rings[(int)RingTypes::Inventory].RingActive = true;
				MenuActive = true;
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
					CombineObjects(item, CombineObject1, CombineObject2);
				}
				else if (CombineTypeFlag == 2)
				{
					CombineTypeFlag = 0;
					ConstructObjectList(item);
					SetupObjectListStartPosition(CombineObject1);
				}
				else if (SeperateTypeFlag)
				{
					SeparateObject(item, Rings[(int)RingTypes::Inventory].CurrentObjectList[Rings[(int)RingTypes::Inventory].CurrentObjectInList].InventoryItem);
				}

				HandleObjectChangeover((int)RingTypes::Inventory);
			}
		}

		int minObj = 0;
		int maxObj = 0;
		int xOffset = 0;
		int n = 0;

		if (ring.NumObjectsInList != 1)
			xOffset = (OBJLIST_SPACING * ring.ObjectListMovement) >> 16;

		if (ring.NumObjectsInList == 2)
		{
			minObj = -1;
			maxObj = 0;
			n = ring.CurrentObjectInList - 1;
		}

		if (ring.NumObjectsInList == 3 || ring.NumObjectsInList == 4)
		{
			minObj = -2;
			maxObj = 1;
			n = ring.CurrentObjectInList - 2;
		}

		if (ring.NumObjectsInList >= 5)
		{
			minObj = -3;
			maxObj = 2;
			n = ring.CurrentObjectInList - 3;
		}

		if (n < 0)
			n += ring.NumObjectsInList;

		if (ring.ObjectListMovement < 0)
			maxObj++;

		if (minObj <= maxObj)
		{
			for (int i = minObj; i <= maxObj; i++)
			{
				int shade = 0;

				if (minObj == i)
				{
					if (ring.ObjectListMovement < 0)
					{
						shade = 0;
					}
					else
					{
						shade = ring.ObjectListMovement >> 9;
					}
				}
				else if (i != minObj + 1 || maxObj == minObj + 1)
				{
					if (i != maxObj)
					{
						shade = 128;
					}
					else
					{
						if (ring.ObjectListMovement < 0)
						{
							shade = (-128 * ring.ObjectListMovement) >> 16;
						}
						else
						{
							shade = 128 - short(ring.ObjectListMovement >> 9);
						}
					}
				}
				else
				{
					if (ring.ObjectListMovement < 0)
					{
						shade = 128 - ((-128 * ring.ObjectListMovement) >> 16);
					}
					else
					{
						shade = 128;
					}
				}

				if (!minObj && !maxObj)
					shade = 128;

				if (ringType == RingTypes::Ammo && CombineRingFadeVal < 128 && shade)
				{
					shade = CombineRingFadeVal;
				}
				else if (ringType == RingTypes::Inventory && NormalRingFadeVal < 128 && shade)
				{
					shade = NormalRingFadeVal;
				}

				auto& listObject = ring.CurrentObjectList[n];
				const auto& invObject = InventoryObjectTable[listObject.InventoryItem];

				if (!i)
				{
					int numItems = 0;
					int count = 0;
					char textBuffer[128];

					switch (invObject.ObjectNumber)
					{
					case ID_BIGMEDI_ITEM:
						numItems = player.Inventory.TotalLargeMedipacks;
						break;

					case ID_SMALLMEDI_ITEM:
						numItems = player.Inventory.TotalSmallMedipacks;
						break;

					case ID_FLARE_INV_ITEM:
						numItems = player.Inventory.TotalFlares;
						break;

					default:
						if (invObject.ObjectNumber < ID_PUZZLE_ITEM1 ||
							invObject.ObjectNumber > ID_PUZZLE_ITEM16)
						{
							switch (invObject.ObjectNumber)
							{
							case ID_SHOTGUN_AMMO1_ITEM:
								numItems = player.Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
								break;

							case ID_SHOTGUN_AMMO2_ITEM:
								numItems = player.Weapons[(int)LaraWeaponType::Shotgun].Ammo[(int)WeaponAmmoType::Ammo2].GetCount();
								break;

							case ID_HK_AMMO_ITEM:
								numItems = player.Weapons[(int)LaraWeaponType::HK].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
								break;

							case ID_CROSSBOW_AMMO1_ITEM:
								count = player.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
								numItems = count;
								break;

							case ID_CROSSBOW_AMMO2_ITEM:
								count = player.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo2].GetCount();
								numItems = count;
								break;

							case ID_CROSSBOW_AMMO3_ITEM:
								count = player.Weapons[(int)LaraWeaponType::Crossbow].Ammo[(int)WeaponAmmoType::Ammo3].GetCount();
								numItems = count;
								break;

							case ID_GRENADE_AMMO1_ITEM:
								count = player.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
								numItems = count;
								break;

							case ID_GRENADE_AMMO2_ITEM:
								count = player.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo2].GetCount();
								numItems = count;
								break;

							case ID_GRENADE_AMMO3_ITEM:
								count = player.Weapons[(int)LaraWeaponType::GrenadeLauncher].Ammo[(int)WeaponAmmoType::Ammo3].GetCount();
								numItems = count;
								break;

							case ID_ROCKET_LAUNCHER_AMMO_ITEM:
								numItems = player.Weapons[(int)LaraWeaponType::RocketLauncher].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
								break;

							case ID_HARPOON_AMMO_ITEM:
								numItems = player.Weapons[(int)LaraWeaponType::HarpoonGun].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
								break;

							case ID_REVOLVER_AMMO_ITEM:
								numItems = player.Weapons[(int)LaraWeaponType::Revolver].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
								break;

							case ID_UZI_AMMO_ITEM:
								numItems = player.Weapons[(int)LaraWeaponType::Uzi].Ammo[(int)WeaponAmmoType::Ammo1].GetCount();
								break;
							}
						}
						else
						{
							numItems = player.Inventory.Puzzles[invObject.ObjectNumber - ID_PUZZLE_ITEM1];
							if (numItems <= 1)
							{
								sprintf(textBuffer, g_GameFlow->GetString(invObject.ObjectName));
							}
							else
							{
								sprintf(textBuffer, "%d x %s", numItems, g_GameFlow->GetString(invObject.ObjectName));
							}
						}

						break;
					}

					if (invObject.ObjectNumber < ID_PUZZLE_ITEM1 ||
						invObject.ObjectNumber > ID_PUZZLE_ITEM16)
					{
						if (numItems)
						{
							if (numItems == -1)
							{
								sprintf(textBuffer, g_GameFlow->GetString(STRING_UNLIMITED), g_GameFlow->GetString(invObject.ObjectName));
							}
							else
							{
								sprintf(textBuffer, "%d x %s", numItems, g_GameFlow->GetString(invObject.ObjectName));
							}
						}
						else
						{
							sprintf(textBuffer, g_GameFlow->GetString(invObject.ObjectName));
						}
					}

					int objectNumber;
					if (ringType == RingTypes::Inventory)
					{
						objectNumber = int(PHD_CENTER_Y - (SCREEN_SPACE_RES.y + 1) * 0.0625 * 2.5);
					}
					else
					{
						objectNumber = int(PHD_CENTER_Y + (SCREEN_SPACE_RES.y + 1) * 0.0625 * 2.0);
					}

					g_Renderer.AddString(PHD_CENTER_X, objectNumber, textBuffer, PRINTSTRING_COLOR_YELLOW, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
				}

				if (!i && !ring.ObjectListMovement)
				{
					if (invObject.RotFlags & INV_ROT_X)
						listObject.Orientation.x += ANGLE(5.0f);

					if (invObject.RotFlags & INV_ROT_Y)
						listObject.Orientation.y += ANGLE(5.0f);

					if (invObject.RotFlags & INV_ROT_Z)
						listObject.Orientation.z += ANGLE(5.0f);
				}
				else
				{
					SpinBack(listObject.Orientation);
				}

				int activeNum = 0;
				if (ring.ObjectListMovement)
				{
					if (ring.ObjectListMovement > 0)
					{
						activeNum = -1;
					}
					else
					{
						activeNum = 1;
					}
				}

				if (i == activeNum)
				{
					if (listObject.Bright < 160)
						listObject.Bright += 16;

					if (listObject.Bright > 160)
						listObject.Bright = 160;
				}
				else
				{
					if (listObject.Bright > 32)
						listObject.Bright -= 16;

					if (listObject.Bright < 32)
						listObject.Bright = 32;
				}

				int x = 400 + xOffset + i * OBJLIST_SPACING;
				int y = 150;
				int y2 = 480; // Combine.
				short objectNumber = ConvertInventoryItemToObject(listObject.InventoryItem);
				float scaler = invObject.Scale1;
				auto& orient = listObject.Orientation;
				int bits = invObject.MeshBits;

				g_Renderer.DrawObjectIn2DSpace(objectNumber, Vector2(x, (ringType == RingTypes::Inventory) ? y : y2), orient, scaler, 1.0f, bits);

				if (++n >= ring.NumObjectsInList)
					n = 0;
			}

			if (ring.RingActive)
			{
				if (ring.NumObjectsInList != 1 && (ringType != RingTypes::Ammo || CombineRingFadeVal == 128))
				{
					if (ring.ObjectListMovement > 0)
						ring.ObjectListMovement += ANGLE(45.0f);

					if (ring.ObjectListMovement < 0)
						ring.ObjectListMovement -= ANGLE(45.0f);

					if (IsHeld(In::Left))
					{
						if (!ring.ObjectListMovement)
						{
							SoundEffect(SFX_TR4_MENU_ROTATE, nullptr, SoundEnvironment::Always);
							ring.ObjectListMovement += ANGLE(45.0f);

							if (AmmoSelectorFlag)
								AmmoSelectorFadeDir = 2;
						}
					}

					if (IsHeld(In::Right))
					{
						if (!ring.ObjectListMovement)
						{
							SoundEffect(SFX_TR4_MENU_ROTATE, nullptr, SoundEnvironment::Always);
							ring.ObjectListMovement -= ANGLE(45.0f);

							if (AmmoSelectorFlag)
								AmmoSelectorFadeDir = 2;
						}
					}

					if (ring.ObjectListMovement < 65536)
					{
						if (ring.ObjectListMovement < -65535)
						{
							ring.CurrentObjectInList++;

							if (ring.CurrentObjectInList >= ring.NumObjectsInList)
								ring.CurrentObjectInList = 0;

							ring.ObjectListMovement = 0;

							if (ringType == RingTypes::Inventory)
								HandleObjectChangeover(0);
						}
					}
					else
					{
						ring.CurrentObjectInList--;

						if (ring.CurrentObjectInList < 0)
							ring.CurrentObjectInList = ring.NumObjectsInList - 1;

						ring.ObjectListMovement = 0;

						if (ringType == RingTypes::Inventory)
							HandleObjectChangeover(0);
					}
				}
			}
		}
	}

	bool GuiController::CallPause()
	{
		g_Renderer.DumpGameScene();
		PauseAllSounds(SoundPauseMode::Pause);
		SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

		g_Gui.SetInventoryMode(InventoryMode::Pause);
		g_Gui.SetMenuToDisplay(Menu::Pause);
		g_Gui.SetSelectedOption(0);

		bool doExitToTitle = false;

		while (g_Gui.GetInventoryMode() == InventoryMode::Pause)
		{
			g_Gui.DrawInventory();
			g_Renderer.Synchronize();

			if (g_Gui.DoPauseMenu(LaraItem) == InventoryResult::ExitToTitle)
			{
				doExitToTitle = true;
				break;
			}
		}

		if (doExitToTitle)
			StopAllSounds();
		else
			ResumeAllSounds(SoundPauseMode::Pause);

		return doExitToTitle;
	}

	bool GuiController::CallInventory(ItemInfo* item, bool resetMode)
	{
		auto* lara = GetLaraInfo(item);

		bool doLoad = false;

		lara->Inventory.OldBusy = lara->Inventory.IsBusy;

		g_Renderer.DumpGameScene();
		PauseAllSounds(SoundPauseMode::Inventory);
		SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);

		if (resetMode)
			SetInventoryMode(InventoryMode::InGame);

		InitializeInventory(item);
		Camera.numberFrames = 2;

		bool exitLoop = false;
		while (!exitLoop)
		{
			if (ThreadEnded)
				return false;

			TimeInMenu++;
			GameTimer++;

			UpdateInputActions(item);

			if (GuiIsDeselected() || IsClicked(In::Inventory))
			{
				SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
				exitLoop = true;
			}

			DrawInventory();
			DrawCompass(item);

			switch (InvMode)
			{
			case InventoryMode::InGame:
				DoInventory(item);
				break;

			case InventoryMode::Statistics:
				DoStatisticsMode();
				break;

			case InventoryMode::Examine:
				DoExamineMode();
				break;

			case InventoryMode::Diary:
				DoDiary(item);
				break;

			case InventoryMode::Load:
				switch (DoLoad())
				{
				case LoadResult::Load:
					doLoad = true;
					exitLoop = true;
					break;

				case LoadResult::Cancel:
					exitLoop = !resetMode;

					if (resetMode)
						SetInventoryMode(InventoryMode::InGame);

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
						SetInventoryMode(InventoryMode::InGame);
				}

				break;
			}

			if (UseItem && NoAction())
				exitLoop = true;

			SetEnterInventory(NO_ITEM);

			Camera.numberFrames = g_Renderer.Synchronize();
		}

		LastInvItem = Rings[(int)RingTypes::Inventory].CurrentObjectList[Rings[(int)RingTypes::Inventory].CurrentObjectInList].InventoryItem;
		UpdateWeaponStatus(item);

		if (UseItem)
			UseCurrentItem(item);

		AlterFOV(LastFOV);
		ResumeAllSounds(SoundPauseMode::Inventory);

		lara->Inventory.IsBusy = lara->Inventory.OldBusy;
		SetInventoryMode(InventoryMode::None);

		return doLoad;
	}

	void GuiController::DoStatisticsMode()
	{
		SetInventoryMode(InventoryMode::Statistics);

		if (GuiIsDeselected())
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			SetInventoryMode(InventoryMode::InGame);
		}
	}

	void GuiController::DoExamineMode()
	{
		SetInventoryMode(InventoryMode::Examine);

		if (GuiIsDeselected())
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			SetInventoryMode(InventoryMode::None);
		}
	}

	void GuiController::DrawCompass(ItemInfo* item)
	{
		constexpr auto POS_2D	  = Vector2(130.0f, 450.0f);
		constexpr auto LERP_ALPHA = 0.1f;

		auto needleOrient = EulerAngles(0, CompassNeedleAngle, 0);
		needleOrient.Lerp(EulerAngles(0, item->Pose.Orientation.y, 0), LERP_ALPHA);

		float wibble = std::sin(((float)(GameTimer & 0x3F) / (float)0x3F) * PI_MUL_2);
		this->CompassNeedleAngle = needleOrient.y + ANGLE(wibble);

		// HACK: Needle is rotated in the draw function.
		const auto& invObject = InventoryObjectTable[INV_OBJECT_COMPASS];
		g_Renderer.DrawObjectIn2DSpace(ID_COMPASS_ITEM, POS_2D, EulerAngles::Zero, invObject.Scale1 * 1.5f);
	}

	void GuiController::DoDiary(ItemInfo* item)
	{
		auto* lara = GetLaraInfo(item);

		SetInventoryMode(InventoryMode::Diary);

		if (GuiIsPulsed(In::Right) &&
			lara->Inventory.Diary.CurrentPage < lara->Inventory.Diary.NumPages)
		{
			lara->Inventory.Diary.CurrentPage++;
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}

		if (GuiIsPulsed(In::Left) &&
			lara->Inventory.Diary.CurrentPage > 1)
		{
			lara->Inventory.Diary.CurrentPage--;
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
		}

		if (GuiIsDeselected())
		{
			SoundEffect(SFX_TR4_MENU_SELECT, nullptr, SoundEnvironment::Always);
			SetInventoryMode(InventoryMode::None);
		}
	}

	int GuiController::GetLoadSaveSelection()
	{
		return SelectedSaveSlot;
	}

	LoadResult GuiController::DoLoad()
	{
		if (GuiIsPulsed(In::Back))
		{
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

			if (SelectedSaveSlot == (SAVEGAME_MAX - 1))
				SelectedSaveSlot -= SAVEGAME_MAX - 1;
			else
				SelectedSaveSlot++;
		}

		if (GuiIsPulsed(In::Forward))
		{
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

			if (SelectedSaveSlot == 0)
				SelectedSaveSlot += SAVEGAME_MAX - 1;
			else
				SelectedSaveSlot--;
		}

		if (GuiIsSelected())
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

		if (GuiIsDeselected() || IsClicked(In::Load))
			return LoadResult::Cancel;

		return LoadResult::None;
	}

	bool GuiController::DoSave()
	{
		if (GuiIsPulsed(In::Back))
		{
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

			if (SelectedSaveSlot == (SAVEGAME_MAX - 1))
				SelectedSaveSlot -= SAVEGAME_MAX - 1;
			else
				SelectedSaveSlot++;
		}

		if (GuiIsPulsed(In::Forward))
		{
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);

			if (SelectedSaveSlot == 0)
				SelectedSaveSlot += SAVEGAME_MAX - 1;
			else
				SelectedSaveSlot--;
		}

		if (GuiIsSelected())
		{
			SoundEffect(SFX_TR4_MENU_CHOOSE, nullptr, SoundEnvironment::Always);
			SaveGame::Save(SelectedSaveSlot);
			return true;
		}

		if (GuiIsDeselected() || IsClicked(In::Save))
			return true;

		return false;
	}

	bool GuiController::PerformWaterskinCombine(ItemInfo* item, bool flag)
	{
		auto* lara = GetLaraInfo(item);

		int smallLiters = lara->Inventory.SmallWaterskin - 1; // How many liters in the small one?
		int bigLiters = lara->Inventory.BigWaterskin - 1;	  // How many liters in the big one?
		int smallCapacity = 3 - smallLiters;				  // How many more liters can fit in the small one?
		int bigCapacity = 5 - bigLiters;					  // How many more liters can fit in the big one?

		int i;
		if (flag)
		{
			// Big one isn't empty and the small one isn't full.
			if (lara->Inventory.BigWaterskin != 1 && smallCapacity)
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

				lara->Inventory.SmallWaterskin = smallLiters + 1;
				lara->Inventory.BigWaterskin = bigLiters + 1;
				CombineObject1 = (smallLiters + 1) + (INV_OBJECT_SMALL_WATERSKIN_EMPTY - 1);
				return true;
			}
		}
		else 
		{
			// Small one isn't empty and the big one isn't full.
			if (lara->Inventory.SmallWaterskin != 1 && bigCapacity)
			{
				i = lara->Inventory.SmallWaterskin - 1;

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

				lara->Inventory.SmallWaterskin = smallLiters + 1;
				lara->Inventory.BigWaterskin = bigLiters + 1;
				CombineObject1 = (bigLiters + 1) + (INV_OBJECT_BIG_WATERSKIN_EMPTY - 1);
				return true;
			}
		}

		return false;
	}
}
