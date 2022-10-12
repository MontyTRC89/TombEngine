#pragma once
#include "Game/GuiObjects.h"
#include "LanguageScript.h"
#include "Specific/configuration.h"

struct ItemInfo;

namespace TEN::Gui
{
	enum class InventoryMode
	{
		None,
		InGame,
		Pause,
		Statistics,
		Examine,
		Diary,
		Load,
		Save
	};

	enum class InventoryResult
	{
		None,
		UseItem,
		NewGame,
		LoadGame,
		SaveGame,
		ExitGame,
		ExitToTitle,
		NewGameSelectedLevel
	};

	enum class LoadResult
	{
		None,
		Load,
		Cancel
	};

	enum class MenuType
	{
		None,
		Use,
		ChooseAmmo,
		Combine,
		Seperate,
		Equip,
		Ammo1,
		Ammo2,
		Ammo3,
		Load,
		Save,
		Examine,
		Statistics,
		Diary
	};

	enum class RingTypes
	{
		Inventory,
		Ammo
	};

	enum class Menu
	{
		Title,
		Pause,
		Statistics,
		SelectLevel,
		LoadGame,
		Options,
		Display,
		Controls,
		OtherSettings
	};

	struct MenuOption
	{
		MenuType	Type;
		char const* Text;
	};

	struct ObjectList
	{
		short InventoryItem;
		unsigned short XRot;
		unsigned short YRot;
		unsigned short ZRot;
		unsigned short Bright;
	};

	struct InventoryRing
	{
		ObjectList CurrentObjectList[INVENTORY_TABLE_SIZE + 1];
		bool RingActive;
		int ObjectListMovement;
		int CurrentObjectInList;
		int NumObjectsInList;
	};

	struct SettingsData
	{
		bool WaitingForKey = false; // Waiting for a key to be pressed when configuring controls.
		bool IgnoreInput = false;   // Ignore input unless all keys were released.
		int SelectedScreenResolution;
		GameConfiguration Configuration;
	};

	class GuiController
	{
	public:
		bool CallInventory(ItemInfo* item, bool resetMode);
		InventoryResult TitleOptions(ItemInfo* item);
		InventoryResult DoPauseMenu(ItemInfo* item);
		void DrawInventory();
		void DrawCurrentObjectList(ItemInfo* item, int ringIndex);
		int IsObjectInInventory(int objectNumber);
		int ConvertObjectToInventoryItem(int objectNumber);
		int ConvertInventoryItemToObject(int objectNumber);
		void FadeAmmoSelector();
		void DrawAmmoSelector();
		bool PerformWaterskinCombine(ItemInfo* item, bool flag);
		void DrawCompass(ItemInfo* item);

		// Getters
		InventoryRing* GetRings(int ringIndex);
		short GetSelectedOption();
		Menu GetMenuToDisplay();
		InventoryMode GetInventoryMode();
		int GetInventoryItemChosen();
		int GetEnterInventory();
		int GetLastInventoryItem();
		SettingsData GetCurrentSettings();
		short GetLoadSaveSelection();

		// Setters
		void SetSelectedOption(int menu);
		void SetMenuToDisplay(Menu menu);
		void SetInventoryMode(InventoryMode mode);
		void SetEnterInventory(int number);
		void SetInventoryItemChosen(int number);

	private:
		#define CAN_SELECT   (!IsHeld(In::Deselect) && \
							 GetActionTimeActive(In::Action) <= GetActionTimeInactive(In::Deselect) && \
							 GetActionTimeActive(In::Action) <= GetActionTimeInactive(In::Save) && \
							 GetActionTimeActive(In::Action) <= GetActionTimeInactive(In::Load) && \
							 GetActionTimeActive(In::Action) <= GetActionTimeInactive(In::Pause))
		#define CAN_DESELECT (!(IsHeld(In::Select) || IsHeld(In::Action)))

		// Input macros
		#define GUI_ACTION_PULSE_UP	   (IsPulsed(In::Forward, 0.1f, 0.4f) && !IsHeld(In::Back))
		#define GUI_ACTION_PULSE_DOWN  (IsPulsed(In::Back, 0.1f, 0.4f) && !IsHeld(In::Forward))
		#define GUI_ACTION_PULSE_LEFT  IsPulsed(In::Left, 0.1f, 0.4f)
		#define GUI_ACTION_PULSE_RIGHT IsPulsed(In::Right, 0.1f, 0.4f)
		#define GUI_ACTION_SELECT	   ((IsReleased(In::Select) || IsReleased(In::Action)) && CAN_SELECT)
		#define GUI_ACTION_DESELECT	   (IsClicked(In::Deselect) && CAN_DESELECT)

		// GUI variables
		Menu MenuToDisplay = Menu::Title;
		int SelectedOption;
		int OptionCount;
		int SelectedSaveSlot;

		SettingsData CurrentSettings;

		// Inventory variables
		short CombineObject1;
		short CombineObject2;
		bool UseItem;
		char SeperateTypeFlag;
		char CombineTypeFlag;
		int CompassNeedleAngle;
		InventoryRing PCRing1;
		InventoryRing PCRing2;
		InventoryRing* Rings[2];
		int CurrentSelectedOption;
		bool MenuActive;
		char AmmoSelectorFlag;
		char NumAmmoSlots;
		char* CurrentAmmoType;
		AmmoList AmmoObjectList[3];
		short AmmoSelectorFadeVal;
		short AmmoSelectorFadeDir;
		short CombineRingFadeVal;
		short CombineRingFadeDir;
		short NormalRingFadeVal;
		short NormalRingFadeDir;
		unsigned char AmmoActive;
		int OBJLIST_SPACING;
		MenuOption CurrentOptions[3];
		InventoryMode InvMode;
		int InventoryItemChosen;
		int EnterInventory;
		int LastInvItem;
		AmmoData Ammo;

		void HandleDisplaySettingsInput(bool fromPauseMenu);
		void HandleControlSettingsInput(ItemInfo* item, bool fromPauseMenu);
		void HandleOtherSettingsInput(bool fromPauseMenu);
		void HandleOptionsInput();
		void BackupOptions();
		bool DoObjectsCombine(int objectNumber1, int objectNumber2);
		void InitialiseInventory(ItemInfo* item);
		void FillDisplayOptions();
		bool IsItemCurrentlyCombinable(int objectNumber);
		bool IsItemInInventory(int objectNumber);
		void CombineObjects(ItemInfo* item, int objectNumber1, int objectNumber2);
		void SetupObjectListStartPosition(int newObjectNumber);
		void SetupObjectListStartPosition2(int newObjectNumber);
		void HandleObjectChangeover(int ringIndex);
		void SetupAmmoSelector();
		void ConstructObjectList(ItemInfo* item);
		void SeparateObject(ItemInfo* item, int objectNumber);
		void InsertObjectIntoList(int objectNumber);
		void InsertObjectIntoList_v2(int objectNumber);
		void UseCurrentItem(ItemInfo* item);
		void SpinBack(unsigned short* angle);
		void UpdateWeaponStatus(ItemInfo* item);
		void DoStatisticsMode();
		void DoExamineMode();
		void DoDiary(ItemInfo* item);
		LoadResult DoLoad();
		bool DoSave();
		void DoInventory(ItemInfo* item);
		void ConstructCombineObjectList(ItemInfo* item);
	};

	extern GuiController g_Gui;
	extern const char* ControlStrings[];
}
