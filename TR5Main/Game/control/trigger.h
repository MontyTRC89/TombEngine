#pragma once

struct ITEM_INFO;
struct FLOOR_INFO;

#define TRIG_BITS(T) ((T & 0x3FFF) >> 10)

constexpr auto ONESHOT     = 0x0100;
constexpr auto SWONESHOT   = 0x0040;
constexpr auto ATONESHOT   = 0x0080;
constexpr auto VALUE_BITS  = 0x03FF;
constexpr auto CODE_BITS   = 0x3E00;
constexpr auto END_BIT     = 0x8000;

enum FLOORDATA_MASKS
{
	FD_MASK_FUNCTION = 0x001F,
	FD_MASK_SUBFUNCTION = 0x7F00,
	FD_MASK_END_DATA = 0x8000
};

enum TRIGGER_TYPES
{
	TRIGGER,
	PAD,
	SWITCH,
	KEY,
	PICKUP,
	HEAVY,
	ANTIPAD,
	COMBAT,
	DUMMY,
	ANTITRIGGER,
	HEAVYSWITCH,
	HEAVYANTITRIGGER,
	MONKEY,
	TIGHTROPE_T,
	CRAWLDUCK_T,
	CLIMB_T
};

enum TRIGOBJECTS_TYPES
{
	TO_OBJECT,
	TO_CAMERA,
	TO_SINK,
	TO_FLIPMAP,
	TO_FLIPON,
	TO_FLIPOFF,
	TO_TARGET,
	TO_FINISH,
	TO_CD,
	TO_FLIPEFFECT,
	TO_SECRET,
	TO_BODYBAG,
	TO_FLYBY,
	TO_CUTSCENE
};

bool GetKeyTrigger(ITEM_INFO* item);
int GetSwitchTrigger(ITEM_INFO* item, short* itemNos, int attatchedToSwitch);
int SwitchTrigger(short itemNum, short timer);
int KeyTrigger(short itemNum);
int PickupTrigger(short itemNum);
int TriggerActive(ITEM_INFO* item);
short* GetTriggerIndex(FLOOR_INFO* floor, int x, int y, int z);
short* GetTriggerIndex(ITEM_INFO* item);
void TestTriggers(short* data, bool heavy, int heavyFlags = 0);
void TestTriggers(int x, int y, int z, short roomNumber, bool heavy, int heavyFlags = 0);
void TestTriggers(ITEM_INFO* item, bool heavy, int heavyFlags = 0);
void ProcessSectorFlags(FLOOR_INFO* floor);
void ProcessSectorFlags(int x, int y, int z, short roomNumber);
void ProcessSectorFlags(ITEM_INFO* item);