#pragma once
#include "configuration.h"
#include <Scripting/LanguageScript.h>

void DrawInv();
void do_debounced_input();
void clear_input_vars(bool flag);
int TitleOptions();
__int64 getTitleSelection();
int getTitleMenu();
void FillDisplayOptions();
void handle_display_setting_input();
void handle_control_settings_input();
void handle_sound_settings_input();
void fillSound();
int DoPauseMenu();
int GetPauseMenu();
__int64 GetPauseSelection();
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
void draw_compass();
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
void combine_KeyItem1(int flag);
void combine_KeyItem2(int flag);
void combine_KeyItem3(int flag);
void combine_KeyItem4(int flag);
void combine_KeyItem5(int flag);
void combine_KeyItem6(int flag);
void combine_KeyItem7(int flag);
void combine_KeyItem8(int flag);
void combine_PickupItem1(int flag);
void combine_PickupItem2(int flag);
void combine_PickupItem3(int flag);
void combine_PickupItem4(int flag);


// Inventory results
#define INV_RESULT_NONE						0
#define INV_RESULT_USE_ITEM					1
#define INV_RESULT_NEW_GAME					2
#define INV_RESULT_LOAD_GAME				3
#define INV_RESULT_SAVE_GAME				4
#define INV_RESULT_EXIT_GAME				5
#define INV_RESULT_EXIT_TO_TILE				6
#define INV_RESULT_NEW_GAME_SELECTED_LEVEL	7

enum item_options
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
	OPT_CHOOSEAMMO_ROCKET = 1 << 17
};

struct uhmG 
{
	int type;
	char* text;
};

struct AMMOLIST 
{
	short	   		invitem;
	short	 		amount;
	unsigned short	yrot;
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

	// Puzzle, keys, pickups, examines
	INV_OBJECT_PUZZLE1,
	INV_OBJECT_PUZZLE2,
	INV_OBJECT_PUZZLE3,
	INV_OBJECT_PUZZLE4,
	INV_OBJECT_PUZZLE5,
	INV_OBJECT_PUZZLE6,
	INV_OBJECT_PUZZLE7,
	INV_OBJECT_PUZZLE8,

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

	INV_OBJECT_KEY1,
	INV_OBJECT_KEY2,
	INV_OBJECT_KEY3,
	INV_OBJECT_KEY4,
	INV_OBJECT_KEY5,
	INV_OBJECT_KEY6,
	INV_OBJECT_KEY7,
	INV_OBJECT_KEY8,

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

	INV_OBJECT_PICKUP1,
	INV_OBJECT_PICKUP2,
	INV_OBJECT_PICKUP3,
	INV_OBJECT_PICKUP4,

	INV_OBJECT_PICKUP1_COMBO1,
	INV_OBJECT_PICKUP1_COMBO2,
	INV_OBJECT_PICKUP2_COMBO1,
	INV_OBJECT_PICKUP2_COMBO2,
	INV_OBJECT_PICKUP3_COMBO1,
	INV_OBJECT_PICKUP3_COMBO2,
	INV_OBJECT_PICKUP4_COMBO1,
	INV_OBJECT_PICKUP4_COMBO2,

	INV_OBJECT_EXAMINE1,
	INV_OBJECT_EXAMINE2,
	INV_OBJECT_EXAMINE3,

	INV_OBJECT_EXAMINE1_COMBO1,
	INV_OBJECT_EXAMINE1_COMBO2,
	INV_OBJECT_EXAMINE2_COMBO1,
	INV_OBJECT_EXAMINE2_COMBO2,
	INV_OBJECT_EXAMINE3_COMBO1,
	INV_OBJECT_EXAMINE3_COMBO2,

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
	IM_STATS
};

typedef struct titleSettings
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
	unsigned short yrot;
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
	void(*combine_routine)(int flag); // size=0, offset=0
	short item1; // size=0, offset=4
	short item2; // size=0, offset=6
	short combined_item; // size=0, offset=8
};

struct INVOBJ
{
	short object_number;
	short yoff;
	short scale1;
	short yrot;
	short xrot;
	short zrot;
	short flags;
	short objname;
	unsigned long meshbits;
};

extern int GLOBAL_invMode;
extern bool pauseMenu;
extern int pause_menu_to_display;
extern __int64 pause_selected_option;
extern int GLOBAL_inventoryitemchosen;
extern int GLOBAL_lastinvitem;
extern int GLOBAL_enterinventory;
extern RINGME pcring1;//items ring
extern RINGME pcring2;//other ring
extern RINGME* rings[2];