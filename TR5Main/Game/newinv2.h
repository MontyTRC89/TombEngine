#pragma once
#include "configuration.h"
#include <Scripting/LanguageScript.h>

void DrawInv();
void do_debounced_input();
void clear_input_vars(bool flag);
int TitleOptions();
void FillDisplayOptions();
void handle_display_setting_input();
void handle_control_settings_input();
void handle_sound_settings_input();
void fillSound();
int DoPauseMenu();
void handle_display_setting_input_pause();
void handle_control_settings_input_pause();
void handle_sound_settings_input_pause();
/*inventory*/
int do_these_objects_combine(int obj1, int obj2);
int is_item_currently_combinable(short obj);
int have_i_got_item(short obj);
void combine_these_two_objects(short obj1, short obj2);
void setup_objectlist_startposition(short newobj);
void handle_object_changeover(int ringnum);
void setup_ammo_selector();
void seperate_object(short obj);
void insert_object_into_list(int num);
void construct_object_list();
void init_inventry();
int have_i_got_object(short object_number);
void setup_objectlist_startposition2(short newobj);
int convert_obj_to_invobj(short obj);
int convert_invobj_to_obj(int obj);
void fade_ammo_selector();
void use_current_item();
void handle_inventry_menu();
void draw_current_object_list(int ringnum);
void draw_ammo_selector();
void spinback(unsigned short* angle);
void update_laras_weapons_status();
int S_CallInventory2();
void do_stats_mode();
void do_examine_mode();
void draw_compass();
void do_diary();
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
int do_special_waterskin_combine_bullshit(int flag);

// Inventory results
#define INV_RESULT_NONE						0
#define INV_RESULT_USE_ITEM					1
#define INV_RESULT_NEW_GAME					2
#define INV_RESULT_LOAD_GAME				3
#define INV_RESULT_SAVE_GAME				4
#define INV_RESULT_EXIT_GAME				5
#define INV_RESULT_EXIT_TO_TILE				6
#define INV_RESULT_NEW_GAME_SELECTED_LEVEL	7

enum menu_types
{
	nothing,
	MENU_TYPE_USE,
	MENU_TYPE_CHOOSEAMMO,
	MENU_TYPE_COMBINE,
	MENU_TYPE_SEPERATE,
	MENU_TYPE_EQUIP,
	MENU_TYPE_AMMO1,
	MENU_TYPE_AMMO2,
	MENU_TYPE_AMMO3,
	MENU_TYPE_LOAD,
	MENU_TYPE_SAVE,
	MENU_TYPE_EXAMINE,
	MENU_TYPE_STATS,
	MENU_TYPE_DIARY
};

enum item_options : int64_t
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

enum rotflags
{
	INV_ROT_X = 1,
	INV_ROT_Y = 2,
	INV_ROT_Z = 4
};

struct uhmG 
{
	int type;
	char const* text;
};

struct AMMOLIST 
{
	short	   		invitem;
	short	 		amount;
	unsigned short	xrot;
	unsigned short	yrot;
	unsigned short	zrot;
};

enum inv_objects
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

enum ring_types
{
	RING_INVENTORY,
	RING_AMMO
};

enum inv_modes
{
	IM_NONE,
	IM_INGAME,
	IM_PAUSE,
	IM_STATS,
	IM_EXAMINE,
	IM_DIARY
};

struct titleSettings
{
	bool waitingForkey;//waiting for a key to be pressed when configuring controls
	int videoMode;
	GameConfiguration conf;
};

extern titleSettings CurrentSettings;

enum title_menus
{
	title_main_menu,
	title_select_level,
	title_load_game,
	title_options_menu,
	title_display_menu,
	title_controls_menu,
	title_sounds_menu
};

enum pause_menus
{
	pause_main_menu,
	pause_statistics,
	pause_options_menu,
	pause_display_menu,
	pause_controls_menu,
	pause_sounds_menu
};

struct OBJLIST
{
	short invitem;
	unsigned short xrot;
	unsigned short yrot;
	unsigned short zrot;
	unsigned short bright;
};

struct RINGME
{
	OBJLIST current_object_list[INVENTORY_TABLE_SIZE + 1];
	int ringactive;
	int objlistmovement;
	int curobjinlist;
	int numobjectsinlist;
};

struct COMBINELIST
{
	void(*combine_routine)(int flag);
	short item1;
	short item2;
	short combined_item;
};

struct INVOBJ
{
	short object_number;
	short yoff;
	float scale1;
	short yrot;
	short xrot;
	short zrot;
	unsigned __int64 opts;
	const char* objname;
	unsigned long meshbits;
	short rot_flags;
};

extern const char* controlmsgs[];
extern int GLOBAL_invMode;
extern int pause_menu_to_display;
extern __int64 pause_selected_option;
extern __int64 title_selected_option;
extern int title_menu_to_display;
extern int GLOBAL_inventoryitemchosen;
extern int GLOBAL_lastinvitem;
extern int GLOBAL_enterinventory;
extern RINGME* rings[2];
extern INVOBJ inventry_objects_list[];
