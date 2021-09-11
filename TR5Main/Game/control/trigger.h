#pragma once

struct ITEM_INFO;
struct FLOOR_INFO;

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
	SKELETON_T,
	TIGHTROPE_T,
	CRAWLDUCK_T,
	CLIMB_T,
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

enum FLOORDATA_MASKS
{
	FD_MASK_FUNCTION = 0x1F,
	FD_MASK_SUBFUNCTION = 0x7F00,
	FD_MASK_END_DATA = 0x8000
};

int GetKeyTrigger(ITEM_INFO* item);
int GetSwitchTrigger(ITEM_INFO* item, short* itemNos, int AttatchedToSwitch);
int SwitchTrigger(short itemNum, short timer);
int TriggerActive(ITEM_INFO* item);
short* GetTriggerIndex(FLOOR_INFO* floor, int x, int y, int z);
short* GetTriggerIndex(ITEM_INFO* item);
void TestTriggers(short* data, bool heavy, int heavyFlags);
void TestTriggers(int x, int y, int z, short roomNumber, bool heavy, int heavyFlags);
void TestTriggers(ITEM_INFO* item, bool heavy, int heavyFlags);
void ProcessSectorFlags(FLOOR_INFO* floor);
void ProcessSectorFlags(int x, int y, int z, short roomNumber);
void ProcessSectorFlags(ITEM_INFO* item);