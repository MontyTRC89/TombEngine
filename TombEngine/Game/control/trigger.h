#pragma once

struct ItemInfo;
class FloorInfo;

#define TRIG_BITS(T) ((T & 0x3FFF) >> 10)

constexpr auto ONESHOT     = 0x0100;
constexpr auto SWONESHOT   = 0x0040;
constexpr auto ATONESHOT   = 0x0080;
constexpr auto VALUE_BITS  = 0x03FF;
constexpr auto CODE_BITS   = 0x3E00;
constexpr auto END_BIT     = 0x8000;
constexpr auto TRIGGERED   = 0x20;
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

extern int TriggerTimer;
extern int KeyTriggerActive;

bool GetKeyTrigger(ItemInfo* item);
int GetSwitchTrigger(ItemInfo* item, short* itemNos, int attatchedToSwitch);
int SwitchTrigger(short itemNumber, short timer);
int KeyTrigger(short itemNum);
int PickupTrigger(short itemNum);
void RefreshCamera(short type, short* data);
int TriggerActive(ItemInfo* item);
short* GetTriggerIndex(FloorInfo* floor, int x, int y, int z);
short* GetTriggerIndex(ItemInfo* item);
void TestTriggers(FloorInfo* floor, int x, int y, int z, bool heavy, int heavyFlags = 0);
void TestTriggers(int x, int y, int z, short roomNumber, bool heavy, int heavyFlags = 0);
void TestTriggers(ItemInfo* item, bool heavy, int heavyFlags = 0);
void ProcessSectorFlags(ItemInfo* item);
