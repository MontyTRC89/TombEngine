#pragma once

#include "..\Specific\config.h"
#include <vector>

using namespace std;

// Legacy stuff
#define ObjectInInventory ((__int32 (__cdecl*)(__int16)) 0x00464360)

void Inject_Inventory();

// New inventory
#define NUM_INVENTORY_OBJECTS_PER_RING	120
#define NUM_INVENTORY_RINGS				5
#define NUM_LEVEL_INVENTORY_RINGS		3

#define INVENTORY_TABLE_SIZE		120

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

// Classic objects
#define INV_OBJECT_UZIS				0	
#define INV_OBJECT_PISTOLS			1
#define INV_OBJECT_SHOTGUN			2
#define INV_OBJECT_REVOLVER			3
#define INV_OBJECT_REVOLVER_LASER	4
#define INV_OBJECT_CROSSBOW			5
#define INV_OBJECT_CROSSBOW_LASER	6
#define INV_OBJECT_HK				7
#define INV_OBJECT_HK_LASER			8
#define INV_OBJECT_SHOTGUN_AMMO1	9
#define INV_OBJECT_SHOTGUN_AMMO2	10
#define INV_OBJECT_HK_AMMO1			13
#define INV_OBJECT_CROSSBOW_AMMO1	15
#define INV_OBJECT_CROSSBOW_AMMO2	16
#define INV_OBJECT_REVOLVER_AMMO	17
#define INV_OBJECT_UZI_AMMO			18
#define INV_OBJECT_PISTOLS_AMMO		19
#define INV_OBJECT_LASERSIGHT		20
#define INV_OBJECT_SILENCER			21
#define INV_OBJECT_LARGE_MEDIPACK	22
#define INV_OBJECT_SMALL_MEDIPACK	23
#define INV_OBJECT_BINOCULARS		24
#define INV_OBJECT_FLARES			25
#define INV_OBJECT_TIMEX			26
#define INV_OBJECT_LOAD_FLOPPY		27
#define INV_OBJECT_SAVE_FLOPPY		28
#define INV_OBJECT_PUZZLE1			29
#define INV_OBJECT_PUZZLE2			30
#define INV_OBJECT_PUZZLE3			31
#define INV_OBJECT_PUZZLE4			32
#define INV_OBJECT_PUZZLE5			33
#define INV_OBJECT_PUZZLE6			34
#define INV_OBJECT_PUZZLE7			35
#define INV_OBJECT_PUZZLE8			36
#define INV_OBJECT_PUZZLE1_COMBO1	37
#define INV_OBJECT_PUZZLE1_COMBO2	38
#define INV_OBJECT_PUZZLE2_COMBO1	39
#define INV_OBJECT_PUZZLE2_COMBO2	40
#define INV_OBJECT_PUZZLE3_COMBO1	41
#define INV_OBJECT_PUZZLE3_COMBO2	42
#define INV_OBJECT_PUZZLE4_COMBO1	43
#define INV_OBJECT_PUZZLE4_COMBO2	44
#define INV_OBJECT_PUZZLE5_COMBO1	45
#define INV_OBJECT_PUZZLE5_COMBO2	46
#define INV_OBJECT_PUZZLE6_COMBO1	47
#define INV_OBJECT_PUZZLE6_COMBO2	48
#define INV_OBJECT_PUZZLE7_COMBO1	49
#define INV_OBJECT_PUZZLE7_COMBO2	50
#define INV_OBJECT_PUZZLE8_COMBO1	51
#define INV_OBJECT_PUZZLE8_COMBO2	52
#define INV_OBJECT_KEY1				53
#define INV_OBJECT_KEY2				54
#define INV_OBJECT_KEY3				55
#define INV_OBJECT_KEY4				56
#define INV_OBJECT_KEY5				57
#define INV_OBJECT_KEY6				58
#define INV_OBJECT_KEY7				59
#define INV_OBJECT_KEY8				60
#define INV_OBJECT_KEY1_COMBO1		61
#define INV_OBJECT_KEY1_COMBO2		62
#define INV_OBJECT_KEY2_COMBO1		63
#define INV_OBJECT_KEY2_COMBO2		64
#define INV_OBJECT_KEY3_COMBO1		65
#define INV_OBJECT_KEY3_COMBO2		66
#define INV_OBJECT_KEY4_COMBO1		67
#define INV_OBJECT_KEY4_COMBO2		68
#define INV_OBJECT_KEY5_COMBO1		69
#define INV_OBJECT_KEY5_COMBO2		70
#define INV_OBJECT_KEY6_COMBO1		71
#define INV_OBJECT_KEY6_COMBO2		72
#define INV_OBJECT_KEY7_COMBO1		73
#define INV_OBJECT_KEY7_COMBO2		74
#define INV_OBJECT_KEY8_COMBO1		75
#define INV_OBJECT_KEY8_COMBO2		76
#define INV_OBJECT_PICKUP1			77
#define INV_OBJECT_PICKUP2			78
#define INV_OBJECT_PICKUP3			79
#define INV_OBJECT_PICKUP4			80
#define INV_OBJECT_PICKUP1_COMBO1	81
#define INV_OBJECT_PICKUP1_COMBO2	82
#define INV_OBJECT_PICKUP2_COMBO1	83
#define INV_OBJECT_PICKUP2_COMBO2	84
#define INV_OBJECT_PICKUP3_COMBO1	85
#define INV_OBJECT_PICKUP3_COMBO2	86
#define INV_OBJECT_PICKUP4_COMBO1	87
#define INV_OBJECT_PICKUP4_COMBO2	88
#define INV_OBJECT_BRUNING_TORCH	89
#define INV_OBJECT_CROWBAR			90
#define INV_OBJECT_EXAMINE1			91
#define INV_OBJECT_EXAMINE2			92
#define INV_OBJECT_EXAMINE3			93
#define INV_OBJECT_WETCLOTH1		93
#define INV_OBJECT_GRAPPLING_GUN	95
#define INV_OBJECT_GRAPPLING_AMMO	96
#define INV_OBJECT_WETCLOTH2		97
#define INV_OBJECT_BOTTLE			98

// New objects for TR3 style inventory
#define INV_OBJECT_PASSAPORT		100
#define INV_OBJECT_KEYS				101
#define INV_OBJECT_SUNGLASSES		102
#define INV_OBJECT_HEADPHONES		103
#define INV_OBJECT_POLAROID			104

// New weapons
#define INV_OBJECT_GRENADE_LAUNCHER	105
#define INV_OBJECT_GRENADE_AMMO1	106
#define INV_OBJECT_GRENADE_AMMO2	107
#define INV_OBJECT_GRENADE_AMMO3	108
#define INV_OBJECT_HARPOON_GUN		109
#define INV_OBJECT_HARPOON_AMMO		110
#define INV_OBJECT_ROCKET_LAUNCHER	111
#define INV_OBJECT_ROCKET_AMMO		112
#define INV_OBJECT_CROSSBOW_AMMO3	113

// New misc objects
#define INV_OBJECT_DIARY			114
#define INV_OBJECT_WATERSKIN1		115
#define INV_OBJECT_WATERSKIN2		116

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
#define INV_DISPLAY_SHADOWS				1
#define INV_DISPLAY_CAUSTICS			2
#define INV_DISPLAY_VOLUMETRIC_FOG		3
#define INV_DISPLAY_APPLY				4
#define INV_DISPLAY_CANCEL				5
#define INV_DISPLAY_COUNT				6

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
#define INV_OBJECTS_SCALE				1.5f
#define INV_SECONDARY_MOVEMENT			200.0f
#define INV_OBJECTS_DISTANCE			2048.0f
#define INV_CAMERA_TILT					5.0f
#define INV_CAMERA_ANIMATION_TILT		60.0f
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

struct InventoryObject {
	__int32 inventoryObject;
	__int32 rotation;
	float scale;
	__int32 distance;
};

struct InventoryRing {
	InventoryObject objects[NUM_INVENTORY_OBJECTS_PER_RING];
	__int32 numObjects;
	__int32 currentObject;
	__int32 rotation;
	__int32 distance;
	__int32 focusState;
	__int32 frameIndex;
	bool draw;
	__int32 y;
	__int32 titleStringIndex;

	// Special fields for settings and passport
	__int32 selectedIndex;
	__int32 passportAction;
	__int32 SelectedVideoMode;
	GameConfiguration Configuration;

	// Special fields for objects
	__int32 actions[3];
	__int32 numActions = 3;
};

struct InventorySecondaryRing {
	InventoryObject objects[NUM_INVENTORY_OBJECTS_PER_RING];
	__int32 numObjects;
	__int32 currentObject;
	__int32 movement;
	__int32 selectedIndex;
};

struct InventoryObjectDefinition {
	__int16 objectNumber;
	__int16 objectName;
	__int32 meshBits;
	__int16 rotY;

	InventoryObjectDefinition(__int16 objNum, __int16 objName, __int32 bits, __int16 rot)
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

struct InventoryObjectCombination {
	__int16 piece1;
	__int16 piece2;
	__int16 combinedObject;
	void(*combineRoutine)(__int32 action);
};

void CombinePuzzle1(__int32 action);
void CombinePuzzle2(__int32 action);
void CombinePuzzle3(__int32 action);
void CombinePuzzle4(__int32 action);
void CombinePuzzle5(__int32 action);
void CombinePuzzle6(__int32 action);
void CombinePuzzle7(__int32 action);
void CombinePuzzle8(__int32 action);
void CombineKey1(__int32 action);
void CombineKey2(__int32 action);
void CombineKey3(__int32 action);
void CombineKey4(__int32 action);
void CombineKey5(__int32 action);
void CombineKey6(__int32 action);
void CombineKey7(__int32 action);
void CombineKey8(__int32 action);
void CombinePickup1(__int32 action);
void CombinePickup2(__int32 action);
void CombinePickup3(__int32 action);
void CombinePickup4(__int32 action);
void CombineRevolverLasersight(__int32 action);
void CombineCrossbowLasersight(__int32 action);

class Inventory
{
private:
	InventoryRing						m_rings[NUM_INVENTORY_RINGS];
	InventorySecondaryRing				m_secondaryRing;
	__int32								m_activeRing;
	__int32								m_movement;
	float								m_deltaMovement;
	InventoryObjectDefinition			m_objectsTable[INVENTORY_TABLE_SIZE];
	__int32								m_type;
	vector<InventoryObjectCombination>	m_combinations;
	__int32								m_activeGui;
	float								m_cameraY;
	float								m_cameraTilt;

public:
	Inventory();
	~Inventory();

	void						Initialise();
	void						InitialiseTitle();
	InventoryRing*				GetRing(__int32 index);
	__int32						GetActiveRing();
	void						SetActiveRing(__int32 index);
	__int32						DoInventory();
	__int32						DoTitleInventory();
	void						InsertObject(__int32 ring, __int32 objectNumber);
	float						GetVerticalOffset();
	void						UseCurrentItem();
	InventoryObjectDefinition*	GetInventoryObject(__int32 index);
	__int32						DoPassport();
	void						DoControlsSettings();
	void						DoGraphicsSettings();
	void						DoSoundSettings();
	__int32						PopupObject();
	__int32						PopoverObject();
	__int32						GetType();
	bool						DoCombine();
	bool						DoSepare();
	void						DoSelectAmmo();
	__int32						DoPuzzle();
	__int32						DoWeapon();
	__int32						DoGenericObject();
	void						DoStatistics();
	void						DoExamine();
	bool						IsCurrentObjectWeapon();
	bool						IsCurrentObjectPuzzle();
	bool						IsCurrentObjectGeneric();
	bool						IsCurrentObjectExamine();
	bool						IsObjectPresentInInventory(__int16 object);
	bool						IsObjectCombinable(__int16 object);
	bool						IsObjectSeparable(__int16 object);
	void						AddCombination(__int16 piece1, __int16 piece2, __int16 combinedObject, void (*f) (__int32));
	__int32						GetActiveGui();
	void						LoadObjects(bool isReload);
	void						SelectObject(__int32 ring, __int32 object, float scale);
	void						OpenRing(__int32 r, bool animateCamera);
	void						CloseRing(__int32 r, bool animateCamera);
	void						SwitchRing(__int32 from, __int32 to, float verticalShift);
	float						GetCameraY();
	float						GetCameraTilt();
};

extern Inventory* g_Inventory;