#pragma once
#include "configuration.h"

// New inventory
#define NUM_INVENTORY_OBJECTS_PER_RING	120
#define NUM_INVENTORY_RINGS				5
#define NUM_LEVEL_INVENTORY_RINGS		3

// Movement directions
#define INV_MOVE_STOPPED			0
#define INV_MOVE_RIGHT				1
#define INV_MOVE_LEFT				2
#define INV_MOVE_UP					3
#define INV_MOVE_DOWN				4

// Rings
#define INV_RING_WEAPONS			1
#define INV_RING_PUZZLES			0
#define INV_RING_OPTIONS			2
#define INV_RING_COMBINE			3
#define INV_RING_CHOOSE_AMMO		4

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

typedef struct titleSettings
{
	bool waitingForkey;//waiting for a key to be pressed when configuring controls
	int videoMode;
	GameConfiguration conf;
};

enum INVENTORY_OBJECTS {
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
	INV_OBJECT_HK_LASER,
	INV_OBJECT_HK_AMMO1,
	INV_OBJECT_HK_AMMO2,
	INV_OBJECT_GRAPPLING_GUN,
	INV_OBJECT_GRAPPLING_AMMO,
	INV_OBJECT_GRENADE_LAUNCHER,
	INV_OBJECT_GRENADE_AMMO1,
	INV_OBJECT_GRENADE_AMMO2,
	INV_OBJECT_GRENADE_AMMO3,
	INV_OBJECT_HARPOON_GUN,
	INV_OBJECT_HARPOON_AMMO,
	INV_OBJECT_ROCKET_LAUNCHER,
	INV_OBJECT_ROCKET_AMMO,
	INV_OBJECT_MAGNUMS,
	INV_OBJECT_MAGNUMS_AMMO,

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
	INV_OBJECT_WETCLOTH1,
	INV_OBJECT_WETCLOTH2,
	INV_OBJECT_BOTTLE,
	INV_OBJECT_CROWBAR,
	INV_OBJECT_DIARY,
	INV_OBJECT_WATERSKIN1,
	INV_OBJECT_WATERSKIN2,

	// Ring inventoory objects
	INV_OBJECT_PASSPORT,
	INV_OBJECT_KEYS,
	INV_OBJECT_SUNGLASSES,
	INV_OBJECT_HEADPHONES,
	INV_OBJECT_POLAROID,

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

typedef struct INVOBJ
{
	short objectNumber;
	short yOff;
	short scale1;
	short yRot;
	short xRot;
	short zRot;
	short flags;
	short objectName;
	int meshBits;
};

// Focus state
#define INV_FOCUS_STATE_NONE		0
#define INV_FOCUS_STATE_POPUP		1
#define INV_FOCUS_STATE_FOCUSED		2	
#define INV_FOCUS_STATE_POPOVER		3

// Passport actions
#define INV_WHAT_NONE					0
#define INV_WHAT_PASSPORT_NEW_GAME		1
#define INV_WHAT_PASSPORT_LOAD_GAME		2
#define INV_WHAT_PASSPORT_SAVE_GAME		3
#define INV_WHAT_PASSPORT_SELECT_LEVEL	4
#define INV_WHAT_PASSPORT_EXIT_GAME		5
#define INV_WHAT_PASSPORT_EXIT_TO_TITLE	6

// Graphics settings
#define INV_DISPLAY_RESOLUTION			0
#define INV_DISPLAY_WINDOWED			1
#define INV_DISPLAY_SHADOWS				2
#define INV_DISPLAY_CAUSTICS			3
#define INV_DISPLAY_VOLUMETRIC_FOG		4
#define INV_DISPLAY_APPLY				5
#define INV_DISPLAY_CANCEL				6
#define INV_DISPLAY_COUNT				7

// Sound settings
#define INV_SOUND_ENABLED				0
#define INV_SOUND_SPECIAL_EFFECTS		1
#define INV_SOUND_MUSIC_VOLUME			2
#define INV_SOUND_SFX_VOLUME			3
#define INV_SOUND_COUNT					4

// Inventory type
#define INV_TYPE_TITLE					0
#define INV_TYPE_GAME					1

// Some rendering parameters
#define INV_RINGS_OFFSET				8192.0f
#define INV_OBJECTS_SCALE				2.0f
#define INV_SECONDARY_MOVEMENT			200.0f
#define INV_OBJECTS_DISTANCE			1792.0f
#define INV_CAMERA_TILT					5.0f
#define INV_CAMERA_ANIMATION_TILT		60.0f
#define INV_OBJECT_TILT					5.0f
#define INV_OBJECT_DISTANCE				512.0f
#define INV_CAMERA_DISTANCE				3072.0f
#define INV_NUM_FRAMES_OPEN_CLOSE		12
#define INV_NUM_FRAMES_ROTATE			8
#define INV_NUM_FRAMES_POPUP			8

// Action for each object
#define INV_ACTION_USE					0
#define INV_ACTION_SELECT_AMMO			1
#define INV_ACTION_COMBINE				2
#define INV_ACTION_SEPARE				3

#define INV_COMBINE_COMBINE				0
#define INV_COMBINE_SEPARE				1

// GUI types
#define INV_GUI_NONE					0
#define INV_GUI_STATISTICS				1
#define INV_GUI_PASSPORT				2
#define INV_GUI_DISPLAY_SETTINGS		3
#define INV_GUI_SOUND_SETTINGS			4
#define INV_GUI_CONTROL_SETTINGS		5
#define INV_GUI_COMBINE					6
#define INV_GUI_SEPARE					7
#define INV_GUI_CHOOSE_AMMO				8

// Inventory results
#define INV_RESULT_NONE						0
#define INV_RESULT_USE_ITEM					1
#define INV_RESULT_NEW_GAME					2
#define INV_RESULT_LOAD_GAME				3
#define INV_RESULT_SAVE_GAME				4
#define INV_RESULT_EXIT_GAME				5
#define INV_RESULT_EXIT_TO_TILE				6
#define INV_RESULT_NEW_GAME_SELECTED_LEVEL	7

#define INV_NUM_COMBINATIONS				22

struct InventoryObject
{
	int inventoryObject;
	int rotation;
	float scale;
};

struct InventoryRing
{
	InventoryObject objects[NUM_INVENTORY_OBJECTS_PER_RING];
	int numObjects;
	int currentObject;
	int rotation;
	int distance;
	int focusState;
	int framePtr;
	bool draw;
	int y;
	int titleStringIndex;

	// Special fields for settings and passport
	int selectedIndex;
	int passportAction;
	int SelectedVideoMode;
	GameConfiguration Configuration;
	bool waitingForKey;

	// Special fields for objects
	int actions[3];
	int numActions = 3;
};

struct InventoryObjectDefinition
{
	short objectNumber;
	short objectName;
	int meshBits;
	int rotY;

	InventoryObjectDefinition(short objNum, short objName, int bits, short rot)
	{
		objectNumber = objNum;
		objectName = objName;
		meshBits = bits;
		rotY = rot;
	}

	InventoryObjectDefinition()
	{

	}
};

struct InventoryObjectCombination
{
	short piece1;
	short piece2;
	short combinedObject;
	void(*combineRoutine)(int action, short index);
};

void CombinePuzzle(int action, short index);
void CombineKey(int action, short index);
void CombinePickup(int action, short index);
void CombineExamine(int action, short index);
void CombineRevolverLasersight(int action, short index);
void CombineCrossbowLasersight(int action, short index);

class Inventory
{
private:
	InventoryRing						m_rings[NUM_INVENTORY_RINGS];
	int									m_activeRing;
	int									m_movement;
	float								m_deltaMovement;
	InventoryObjectDefinition			m_objectsTable[INVENTORY_TABLE_SIZE];
	int									m_type;
	std::vector<InventoryObjectCombination>	m_combinations;
	int									m_activeGui;
	float								m_cameraY;
	float								m_cameraTilt;
	short								m_enterObject;
	short								m_selectedObject;

public:
	Inventory();
	~Inventory();

	void						Initialise();
	void						InitialiseTitle();
	InventoryRing*				GetRing(int index);
	int							GetActiveRing();
	void						SetActiveRing(int index);
	int							DoInventory();
	int							DoTitleInventory();
	void						InsertObject(int ring, int objectNumber);
	float						GetVerticalOffset();
	void						UseCurrentItem();
	InventoryObjectDefinition*	GetInventoryObject(int index);
	int							DoPassport();
	void						DoControlsSettings();
	void						DoGraphicsSettings();
	void						DoSoundSettings();
	int							PopupObject();
	int							PopoverObject();
	int							GetType();
	bool						DoCombine();
	bool						DoSepare();
	void						DoSelectAmmo();
	int							DoPuzzle();
	int							DoWeapon();
	int							DoGenericObject();
	void						DoStatistics();
	void						DoExamine();
	bool						IsCurrentObjectWeapon();
	bool						IsCurrentObjectPuzzle();
	bool						IsCurrentObjectGeneric();
	bool						IsCurrentObjectExamine();
	bool						IsInventoryObjectPresentInInventory(short object);
	bool						IsObjectPresentInInventory(short object);
	int							FindObjectRing(short object);
	int							FindObjectIndex(short object);
	bool						IsObjectCombinable(short object);
	bool						IsObjectSeparable(short object);
	void						AddCombination(short piece1, short piece2, short combinedObject, void (*f) (int, short));
	int							GetActiveGui();
	void						LoadObjects(bool isReload);
	void						SelectObject(int ring, int object, float scale);
	void						OpenRing(int r, bool animateCamera);
	void						CloseRing(int r, bool animateCamera);
	void						SwitchRing(int from, int to, float verticalShift);
	float						GetCameraY();
	float						GetCameraTilt();
	bool						HasWeaponMultipleAmmos(short object);
	bool						UpdateSceneAndDrawInventory();
	short						GetEnterObject();
	short						GetSelectedObject();
	void						SetEnterObject(short objNum);
	void						SetSelectedObject(short objNum);
	/*start troye's lame shit*/
	void						do_debounced_input();
	void						clear_input_vars(bool flag);
	int							TitleOptions();
	__int64						getTitleSelection();
	int							getTitleMenu();
	void						FillDisplayOptions();
	void						handle_display_setting_input();
	void						handle_control_settings_input();
	void						handle_sound_settings_input();
	void						fillSound();
};

extern Inventory g_Inventory;
extern titleSettings CurrentSettings;