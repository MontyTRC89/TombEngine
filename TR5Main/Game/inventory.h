#pragma once

// Legacy stuff
#define InitialiseInventory ((void (__cdecl*)()) 0x0045FEF0)

void Inject_Inventory();

// New inventory
#define NUM_INVENTORY_OBJECTS_PER_RING	120
#define NUM_INVENTORY_RINGS				3
#define NUM_LEVEL_INVENTORY_RINGS		3

#define INVENTORY_TABLE_SIZE		120

#define INV_MOVE_STOPPED			0
#define INV_MOVE_RIGHT				1
#define INV_MOVE_LEFT				2
#define INV_MOVE_UP					3
#define INV_MOVE_DOWN				4

#define INV_RING_WEAPONS			1
#define INV_RING_PUZZLES			0
#define INV_RING_OPTIONS			2

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
#define INV_OBJECT_GRENADE_AMMO		106
#define INV_OBJECT_HARPOON_GUN		107
#define INV_OBJECT_HARPOON_AMMO		108
#define INV_OBJECT_ROCKET_LAUNCHER	109
#define INV_OBJECT_ROCKET_AMMO		110

#define INV_RESULT_NEW_GAME			0
#define INV_RESULT_LOAD_GAME		1
#define INV_RESULT_LARA_HOME		2
#define INV_RESULT_EXIT				4

#define INV_FOCUS_STATE_NONE		0
#define INV_FOCUS_STATE_POPUP		1
#define INV_FOCUS_STATE_FOCUSED		2	
#define INV_FOCUS_STATE_POPOVER		3

#define INV_WHAT_NONE					0
#define INV_WHAT_PASSPORT_NEW_GAME		1
#define INV_WHAT_PASSPORT_LOAD_GAME		2
#define INV_WHAT_PASSPORT_SAVE_GAME		3
#define INV_WHAT_PASSPORT_SELECT_LEVEL	4
#define INV_WHAT_PASSPORT_EXIT_GAME		5
#define INV_WHAT_PASSPORT_EXIT_TO_TITLE	6
#define INV_WHAT_DISPLAY_SETTINGS		7
#define INV_WHAT_AUDIO_SETTINGS			8
#define INV_WHAT_CONTROLS_SETTINGS		9

#define INV_TYPE_TITLE					0
#define INV_TYPE_GAME					1

#define INV_RINGS_OFFSET				8192.0f
#define INV_OBJECT_SCALE				1.5f

typedef enum INVENTORY_RESULT {
	INVENTORY_RESULT_NONE,
	INVENTORY_RESULT_USE_ITEM,
	INVENTORY_RESULT_NEW_GAME,
	INVENTORY_RESULT_LOAD_GAME,
	INVENTORY_RESULT_SAVE_GAME,
	INVENTORY_RESULT_EXIT_GAME,
	INVENTORY_RESULT_EXIT_TO_TILE,
	INVENTORY_RESULT_NEW_GAME_SELECTED_LEVEL
};

typedef struct InventoryObject {
	__int32 inventoryObject;
	__int32 rotation;
	float scale;
};

typedef struct InventoryRing {
	InventoryObject objects[NUM_INVENTORY_OBJECTS_PER_RING];
	__int32 numObjects;
	__int32 currentObject;
	__int32 movement;
	__int32 focusState;
	__int32 frameIndex;
	bool draw;

	// Special fields for settings and passport
	__int32 selectedIndex;
	__int32 passportAction;
};

typedef struct InventoryObjectDefinition {
	__int16 objectNumber;
	__int16 objectName;
	__int32 meshBits;
	__int16 rotY;
};

class Inventory
{
private:
	InventoryRing				m_rings[NUM_INVENTORY_RINGS];
	__int32						m_activeRing;
	__int32						m_movement;
	float						m_deltaMovement;
	InventoryObjectDefinition	m_objectsTable[INVENTORY_TABLE_SIZE];
	__int32						m_type;

public:
	Inventory();
	~Inventory();
	void						Initialise();
	void						InitialiseTitle();
	InventoryRing*				GetRing(__int32 index);
	__int32						GetActiveRing();
	void						SetActiveRing(__int32 index);
	INVENTORY_RESULT			DoInventory();
	INVENTORY_RESULT			DoTitleInventory();
	void						InsertObject(__int32 ring, __int32 objectNumber);
	float						GetVerticalOffset();
	void						UseCurrentItem();
	InventoryObjectDefinition*	GetInventoryObject(__int32 index);
	INVENTORY_RESULT			DoPassport();
	void						DoControlsSettings();
	void						DoGraphicsSettings();
	void						DoSoundSettings();
	__int32						PopupObject();
	__int32						PopoverObject();
	__int32						GetType();
};

extern Inventory* g_Inventory;