#pragma once
#include "LanguageScript.h"
#include "Specific/configuration.h"

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

enum ItemOptions : uint64_t
{
	OPT_ALWAYSCOMBINE = 1 << 0,
	OPT_EQUIP = 1 << 1,
	OPT_USE = 1 << 2,
	OPT_COMBINABLE = 1 << 3,
	OPT_SEPERATABLE = 1 << 4,
	OPT_EXAMINABLE = 1 << 5,
	OPT_CHOOSEAMMO_SHOTGUN = 1 << 6,
	OPT_CHOOSEAMMO_CROSSBOW = 1 << 7,
	OPT_CHOOSEAMMO_GRENADEGUN = 1 << 8,
	OPT_CHOOSEAMMO_UZI = 1 << 9,
	OPT_CHOOSEAMMO_PISTOLS = 1 << 10,
	OPT_CHOOSEAMMO_REVOLVER = 1 << 11,
	OPT_LOAD = 1 << 12,
	OPT_SAVE = 1 << 13,
	OPT_CHOOSEAMMO_HK = 1 << 14,
	OPT_STATS = 1 << 15,
	OPT_CHOOSEAMMO_HARPOON = 1 << 16,
	OPT_CHOOSEAMMO_ROCKET = 1 << 17,
	OPT_DIARY = 1 << 18
};

enum RotationFlags
{
	INV_ROT_X = 1,
	INV_ROT_Y = 2,
	INV_ROT_Z = 4
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

enum InventoryObjectTypes
{
	// Weapons and ammos
	INV_OBJECT_PISTOLS,
	INV_OBJECT_PISTOLS_AMMO,
	INV_OBJECT_UZIS,
	INV_OBJECT_UZI_AMMO,
	INV_OBJECT_SHOTGUN,
	INV_OBJECT_SHOTGUN_AMMO1,
	INV_OBJECT_SHOTGUN_AMMO2,
	INV_OBJECT_REVOLVER,
	INV_OBJECT_REVOLVER_AMMO,
	INV_OBJECT_REVOLVER_LASER,
	INV_OBJECT_CROSSBOW,
	INV_OBJECT_CROSSBOW_LASER,
	INV_OBJECT_CROSSBOW_AMMO1,
	INV_OBJECT_CROSSBOW_AMMO2,
	INV_OBJECT_CROSSBOW_AMMO3,
	INV_OBJECT_HK,
	INV_OBJECT_HK_SILENCER,
	INV_OBJECT_HK_AMMO,
	INV_OBJECT_GRENADE_LAUNCHER,
	INV_OBJECT_GRENADE_AMMO1,
	INV_OBJECT_GRENADE_AMMO2,
	INV_OBJECT_GRENADE_AMMO3,
	INV_OBJECT_HARPOON_GUN,
	INV_OBJECT_HARPOON_AMMO,
	INV_OBJECT_ROCKET_LAUNCHER,
	INV_OBJECT_ROCKET_AMMO,

	// Misc objects
	INV_OBJECT_LASERSIGHT,
	INV_OBJECT_SILENCER,
	INV_OBJECT_LARGE_MEDIPACK,
	INV_OBJECT_SMALL_MEDIPACK,
	INV_OBJECT_BINOCULARS,
	INV_OBJECT_FLARES,
	INV_OBJECT_TIMEX,
	INV_OBJECT_LOAD_FLOPPY,
	INV_OBJECT_SAVE_FLOPPY,
	INV_OBJECT_BRUNING_TORCH,
	INV_OBJECT_CROWBAR,
	INV_OBJECT_DIARY,
	INV_OBJECT_COMPASS,
	INV_OBJECT_BEETLE,
	INV_OBJECT_BEETLE_PART1,
	INV_OBJECT_BEETLE_PART2,
	INV_OBJECT_SMOL_WATERSKIN,
	INV_OBJECT_SMOL_WATERSKIN1L,
	INV_OBJECT_SMOL_WATERSKIN2L,
	INV_OBJECT_SMOL_WATERSKIN3L,
	INV_OBJECT_BIG_WATERSKIN,
	INV_OBJECT_BIG_WATERSKIN1L,
	INV_OBJECT_BIG_WATERSKIN2L,
	INV_OBJECT_BIG_WATERSKIN3L,
	INV_OBJECT_BIG_WATERSKIN4L,
	INV_OBJECT_BIG_WATERSKIN5L,
	INV_OBJECT_OPEN_DIARY,

	// Puzzle, keys, pickups, examines
	INV_OBJECT_PUZZLE1,
	INV_OBJECT_PUZZLE2,
	INV_OBJECT_PUZZLE3,
	INV_OBJECT_PUZZLE4,
	INV_OBJECT_PUZZLE5,
	INV_OBJECT_PUZZLE6,
	INV_OBJECT_PUZZLE7,
	INV_OBJECT_PUZZLE8,
	INV_OBJECT_PUZZLE9,
	INV_OBJECT_PUZZLE10,
	INV_OBJECT_PUZZLE11,
	INV_OBJECT_PUZZLE12,
	INV_OBJECT_PUZZLE13,
	INV_OBJECT_PUZZLE14,
	INV_OBJECT_PUZZLE15,
	INV_OBJECT_PUZZLE16,

	INV_OBJECT_PUZZLE1_COMBO1,
	INV_OBJECT_PUZZLE1_COMBO2,
	INV_OBJECT_PUZZLE2_COMBO1,
	INV_OBJECT_PUZZLE2_COMBO2,
	INV_OBJECT_PUZZLE3_COMBO1,
	INV_OBJECT_PUZZLE3_COMBO2,
	INV_OBJECT_PUZZLE4_COMBO1,
	INV_OBJECT_PUZZLE4_COMBO2,
	INV_OBJECT_PUZZLE5_COMBO1,
	INV_OBJECT_PUZZLE5_COMBO2,
	INV_OBJECT_PUZZLE6_COMBO1,
	INV_OBJECT_PUZZLE6_COMBO2,
	INV_OBJECT_PUZZLE7_COMBO1,
	INV_OBJECT_PUZZLE7_COMBO2,
	INV_OBJECT_PUZZLE8_COMBO1,
	INV_OBJECT_PUZZLE8_COMBO2,
	INV_OBJECT_PUZZLE9_COMBO1,
	INV_OBJECT_PUZZLE9_COMBO2,
	INV_OBJECT_PUZZLE10_COMBO1,
	INV_OBJECT_PUZZLE10_COMBO2,
	INV_OBJECT_PUZZLE11_COMBO1,
	INV_OBJECT_PUZZLE11_COMBO2,
	INV_OBJECT_PUZZLE12_COMBO1,
	INV_OBJECT_PUZZLE12_COMBO2,
	INV_OBJECT_PUZZLE13_COMBO1,
	INV_OBJECT_PUZZLE13_COMBO2,
	INV_OBJECT_PUZZLE14_COMBO1,
	INV_OBJECT_PUZZLE14_COMBO2,
	INV_OBJECT_PUZZLE15_COMBO1,
	INV_OBJECT_PUZZLE15_COMBO2,
	INV_OBJECT_PUZZLE16_COMBO1,
	INV_OBJECT_PUZZLE16_COMBO2,

	INV_OBJECT_KEY1,
	INV_OBJECT_KEY2,
	INV_OBJECT_KEY3,
	INV_OBJECT_KEY4,
	INV_OBJECT_KEY5,
	INV_OBJECT_KEY6,
	INV_OBJECT_KEY7,
	INV_OBJECT_KEY8,
	INV_OBJECT_KEY9,
	INV_OBJECT_KEY10,
	INV_OBJECT_KEY11,
	INV_OBJECT_KEY12,
	INV_OBJECT_KEY13,
	INV_OBJECT_KEY14,
	INV_OBJECT_KEY15,
	INV_OBJECT_KEY16,

	INV_OBJECT_KEY1_COMBO1,
	INV_OBJECT_KEY1_COMBO2,
	INV_OBJECT_KEY2_COMBO1,
	INV_OBJECT_KEY2_COMBO2,
	INV_OBJECT_KEY3_COMBO1,
	INV_OBJECT_KEY3_COMBO2,
	INV_OBJECT_KEY4_COMBO1,
	INV_OBJECT_KEY4_COMBO2,
	INV_OBJECT_KEY5_COMBO1,
	INV_OBJECT_KEY5_COMBO2,
	INV_OBJECT_KEY6_COMBO1,
	INV_OBJECT_KEY6_COMBO2,
	INV_OBJECT_KEY7_COMBO1,
	INV_OBJECT_KEY7_COMBO2,
	INV_OBJECT_KEY8_COMBO1,
	INV_OBJECT_KEY8_COMBO2,
	INV_OBJECT_KEY9_COMBO1,
	INV_OBJECT_KEY9_COMBO2,
	INV_OBJECT_KEY10_COMBO1,
	INV_OBJECT_KEY10_COMBO2,
	INV_OBJECT_KEY11_COMBO1,
	INV_OBJECT_KEY11_COMBO2,
	INV_OBJECT_KEY12_COMBO1,
	INV_OBJECT_KEY12_COMBO2,
	INV_OBJECT_KEY13_COMBO1,
	INV_OBJECT_KEY13_COMBO2,
	INV_OBJECT_KEY14_COMBO1,
	INV_OBJECT_KEY14_COMBO2,
	INV_OBJECT_KEY15_COMBO1,
	INV_OBJECT_KEY15_COMBO2,
	INV_OBJECT_KEY16_COMBO1,
	INV_OBJECT_KEY16_COMBO2,

	INV_OBJECT_PICKUP1,
	INV_OBJECT_PICKUP2,
	INV_OBJECT_PICKUP3,
	INV_OBJECT_PICKUP4,
	INV_OBJECT_PICKUP5,
	INV_OBJECT_PICKUP6,
	INV_OBJECT_PICKUP7,
	INV_OBJECT_PICKUP8,
	INV_OBJECT_PICKUP9,
	INV_OBJECT_PICKUP10,
	INV_OBJECT_PICKUP11,
	INV_OBJECT_PICKUP12,
	INV_OBJECT_PICKUP13,
	INV_OBJECT_PICKUP14,
	INV_OBJECT_PICKUP15,
	INV_OBJECT_PICKUP16,

	INV_OBJECT_PICKUP1_COMBO1,
	INV_OBJECT_PICKUP1_COMBO2,
	INV_OBJECT_PICKUP2_COMBO1,
	INV_OBJECT_PICKUP2_COMBO2,
	INV_OBJECT_PICKUP3_COMBO1,
	INV_OBJECT_PICKUP3_COMBO2,
	INV_OBJECT_PICKUP4_COMBO1,
	INV_OBJECT_PICKUP4_COMBO2,
	INV_OBJECT_PICKUP5_COMBO1,
	INV_OBJECT_PICKUP5_COMBO2,
	INV_OBJECT_PICKUP6_COMBO1,
	INV_OBJECT_PICKUP6_COMBO2,
	INV_OBJECT_PICKUP7_COMBO1,
	INV_OBJECT_PICKUP7_COMBO2,
	INV_OBJECT_PICKUP8_COMBO1,
	INV_OBJECT_PICKUP8_COMBO2,
	INV_OBJECT_PICKUP9_COMBO1,
	INV_OBJECT_PICKUP9_COMBO2,
	INV_OBJECT_PICKUP10_COMBO1,
	INV_OBJECT_PICKUP10_COMBO2,
	INV_OBJECT_PICKUP11_COMBO1,
	INV_OBJECT_PICKUP11_COMBO2,
	INV_OBJECT_PICKUP12_COMBO1,
	INV_OBJECT_PICKUP12_COMBO2,
	INV_OBJECT_PICKUP13_COMBO1,
	INV_OBJECT_PICKUP13_COMBO2,
	INV_OBJECT_PICKUP14_COMBO1,
	INV_OBJECT_PICKUP14_COMBO2,
	INV_OBJECT_PICKUP15_COMBO1,
	INV_OBJECT_PICKUP15_COMBO2,
	INV_OBJECT_PICKUP16_COMBO1,
	INV_OBJECT_PICKUP16_COMBO2,

	INV_OBJECT_EXAMINE1,
	INV_OBJECT_EXAMINE2,
	INV_OBJECT_EXAMINE3,
	INV_OBJECT_EXAMINE4,
	INV_OBJECT_EXAMINE5,
	INV_OBJECT_EXAMINE6,
	INV_OBJECT_EXAMINE7,
	INV_OBJECT_EXAMINE8,


	INV_OBJECT_EXAMINE1_COMBO1,
	INV_OBJECT_EXAMINE1_COMBO2,
	INV_OBJECT_EXAMINE2_COMBO1,
	INV_OBJECT_EXAMINE2_COMBO2,
	INV_OBJECT_EXAMINE3_COMBO1,
	INV_OBJECT_EXAMINE3_COMBO2,
	INV_OBJECT_EXAMINE4_COMBO1,
	INV_OBJECT_EXAMINE4_COMBO2,
	INV_OBJECT_EXAMINE5_COMBO1,
	INV_OBJECT_EXAMINE5_COMBO2,
	INV_OBJECT_EXAMINE6_COMBO1,
	INV_OBJECT_EXAMINE6_COMBO2,
	INV_OBJECT_EXAMINE7_COMBO1,
	INV_OBJECT_EXAMINE7_COMBO2,
	INV_OBJECT_EXAMINE8_COMBO1,
	INV_OBJECT_EXAMINE8_COMBO2,

	INVENTORY_TABLE_SIZE
};

struct MenuOption
{
	MenuType type;
	char const* text;
};

struct ObjectList
{
	short invitem;
	unsigned short xrot;
	unsigned short yrot;
	unsigned short zrot;
	unsigned short bright;
};

struct InventoryRing
{
	ObjectList current_object_list[INVENTORY_TABLE_SIZE + 1];
	int ringactive;
	int objlistmovement;
	int curobjinlist;
	int numobjectsinlist;
};

struct AmmoList
{
	short	   		invitem;
	short	 		amount;
	unsigned short	xrot;
	unsigned short	yrot;
	unsigned short	zrot;
};

struct SettingsData
{
	bool waitingForkey = false; // Waiting for a key to be pressed when configuring controls
	bool ignoreInput = false;   // Ignore input unless all keys were released
	int selectedScreenResolution;
	GameConfiguration conf;
};

struct CombineList
{
	void (*combine_routine)(int flag);
	short item1;
	short item2;
	short combined_item;
};

struct InventoryObject
{
	short object_number;
	short yoff;
	float scale1;
	short yrot;
	short xrot;
	short zrot;
	unsigned __int64 opts;
	const char* objname;
	unsigned int meshbits;
	short rot_flags;
};

class GuiController
{
public:
	bool CallInventory(bool reset_mode);
	InventoryResult TitleOptions();
	InventoryResult DoPauseMenu();
	void DrawInventory();
	void DrawCurrentObjectList(int ringnum);
	int IsObjectInInventory(short object_number);
	int ConvertObjectToInventoryItem(short obj);
	int ConvertInventoryItemToObject(int obj);
	void FadeAmmoSelector();
	void DrawAmmoSelector();
	bool PerformWaterskinCombine(int flag);
	void DrawCompass();

	// Getters
	InventoryRing* GetRings(char num);
	short GetSelectedOption();
	Menu GetMenuToDisplay();
	InventoryMode GetInventoryMode();
	int GetInventoryItemChosen();
	int GetEnterInventory();
	int GetLastInventoryItem();
	SettingsData& GetCurrentSettings();
	short GetLoadSaveSelection();

	// Setters
	void SetSelectedOption(short menu);
	void SetMenuToDisplay(Menu menu);
	void SetInventoryMode(InventoryMode mode);
	void SetEnterInventory(int num);
	void SetInventoryItemChosen(int num);

private:
	void DoDebouncedInput();
	void ClearInputVariables(bool flag);
	void HandleDisplaySettingsInput(bool pause);
	void HandleControlSettingsInput(bool pause);
	void HandleOtherSettingsInput(bool pause);
	void HandleOptionsInput();
	void BackupOptions();
	bool DoObjectsCombine(int obj1, int obj2);
	void InitialiseInventory();
	void FillDisplayOptions();
	bool IsItemCurrentlyCombinable(short obj);
	bool IsItemInInventory(short obj);
	void CombineObjects(short obj1, short obj2);
	void SetupObjectListStartPosition(short newobj);
	void SetupObjectListStartPosition2(short newobj);
	void HandleObjectChangeover(int ringnum);
	void SetupAmmoSelector();
	void ConstructObjectList();
	void SeparateObject(short obj);
	void InsertObjectIntoList(int num);
	void InsertObjectIntoList_v2(int num);
	void UseCurrentItem();
	void SpinBack(unsigned short* angle);
	void UpdateWeaponStatus();
	void DoStatisticsMode();
	void DoExamineMode();
	void DoDiary();
	LoadResult DoLoad();
	bool DoSave();
	void DoInventory();
	void ConstructCombineObjectList();
	
	/*vars*/

	// Input
	bool goUp, goDown, goRight, goLeft, goSelect, goDeselect;
	bool dbUp, dbDown, dbRight, dbLeft, dbSelect, dbDeselect;
	long rptRight, rptLeft;

	// Inventory
	short combine_obj1;
	short combine_obj2;
	char useItem;
	char seperate_type_flag;
	char combine_type_flag;
	int compassNeedleAngle;
	InventoryRing pcring1;
	InventoryRing pcring2;
	InventoryRing* rings[2];
	int current_selected_option;
	int menu_active;
	char ammo_selector_flag;
	char num_ammo_slots;
	char* current_ammo_type;
	AmmoList ammo_object_list[3];
	short ammo_selector_fade_val;
	short ammo_selector_fade_dir;
	short combine_ring_fade_val;
	short combine_ring_fade_dir;
	short normal_ring_fade_val;
	short normal_ring_fade_dir;
	unsigned char ammo_active;
	int OBJLIST_SPACING;
	MenuOption current_options[3];
	InventoryMode invMode;
	int inventoryItemChosen;
	int enterInventory;
	int lastInvItem;

	// Ammo vars
	unsigned short AmountShotGunAmmo1;
	unsigned short AmountShotGunAmmo2;
	unsigned short AmountHKAmmo1;
	unsigned short AmountCrossBowAmmo1;
	unsigned short AmountCrossBowAmmo2;
	unsigned short AmountCrossBowAmmo3;
	unsigned short AmountGrenadeAmmo1;
	unsigned short AmountGrenadeAmmo2;
	unsigned short AmountGrenadeAmmo3;
	unsigned short AmountRocketsAmmo;
	unsigned short AmountHarpoonAmmo;
	unsigned short AmountUziAmmo;
	unsigned short AmountRevolverAmmo;
	unsigned short AmountPistolsAmmo;
	char CurrentPistolsAmmoType;
	char CurrentUziAmmoType;
	char CurrentRevolverAmmoType;
	char CurrentShotGunAmmoType;
	char CurrentHKAmmoType;
	char CurrentGrenadeGunAmmoType;
	char CurrentCrossBowAmmoType;
	char CurrentHarpoonAmmoType;
	char CurrentRocketAmmoType;
	char StashedCurrentPistolsAmmoType;
	char StashedCurrentUziAmmoType;
	char StashedCurrentRevolverAmmoType;
	char StashedCurrentShotGunAmmoType;
	char StashedCurrentGrenadeGunAmmoType;
	char StashedCurrentCrossBowAmmoType;
	char Stashedcurrent_selected_option;
	char StashedCurrentHKAmmoType;
	char StashedCurrentHarpoonAmmoType;
	char StashedCurrentRocketAmmoType;

	// GUI vars
	Menu menu_to_display = Menu::Title;
	int selected_option;
	int option_count;
	int selected_save_slot;

	SettingsData CurrentSettings;
};

/*inventory*/
void combine_revolver_lasersight(int flag);
void combine_crossbow_lasersight(int flag);
void combine_HK_SILENCER(int flag);
void combine_PuzzleItem1(int flag);
void combine_PuzzleItem2(int flag);
void combine_PuzzleItem3(int flag);
void combine_PuzzleItem4(int flag);
void combine_PuzzleItem5(int flag);
void combine_PuzzleItem6(int flag);
void combine_PuzzleItem7(int flag);
void combine_PuzzleItem8(int flag);
void combine_PuzzleItem9(int flag);
void combine_PuzzleItem10(int flag);
void combine_PuzzleItem11(int flag);
void combine_PuzzleItem12(int flag);
void combine_PuzzleItem13(int flag);
void combine_PuzzleItem14(int flag);
void combine_PuzzleItem15(int flag);
void combine_PuzzleItem16(int flag);
void combine_KeyItem1(int flag);
void combine_KeyItem2(int flag);
void combine_KeyItem3(int flag);
void combine_KeyItem4(int flag);
void combine_KeyItem5(int flag);
void combine_KeyItem6(int flag);
void combine_KeyItem7(int flag);
void combine_KeyItem8(int flag);
void combine_KeyItem9(int flag);
void combine_KeyItem10(int flag);
void combine_KeyItem11(int flag);
void combine_KeyItem12(int flag);
void combine_KeyItem13(int flag);
void combine_KeyItem14(int flag);
void combine_KeyItem15(int flag);
void combine_KeyItem16(int flag);
void combine_PickupItem1(int flag);
void combine_PickupItem2(int flag);
void combine_PickupItem3(int flag);
void combine_PickupItem4(int flag);
void combine_PickupItem5(int flag);
void combine_PickupItem6(int flag);
void combine_PickupItem7(int flag);
void combine_PickupItem8(int flag);
void combine_PickupItem9(int flag);
void combine_PickupItem10(int flag);
void combine_PickupItem11(int flag);
void combine_PickupItem12(int flag);
void combine_PickupItem13(int flag);
void combine_PickupItem14(int flag);
void combine_PickupItem15(int flag);
void combine_PickupItem16(int flag);
void combine_Examine1(int flag);
void combine_Examine2(int flag);
void combine_Examine3(int flag);
void combine_Examine4(int flag);
void combine_Examine5(int flag);
void combine_Examine6(int flag);
void combine_Examine7(int flag);
void combine_Examine8(int flag);
void combine_ClockWorkBeetle(int flag);

extern GuiController g_Gui;
extern InventoryObject inventry_objects_list[];
extern const char* controlmsgs[];
