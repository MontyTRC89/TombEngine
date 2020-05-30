#pragma once
#include "phd_global.h"

typedef enum AIObjectType
{
	NO_AI = 0x0000,
	GUARD = 0x0001,
	AMBUSH = 0x0002,
	PATROL1 = 0x0004,
	MODIFY = 0x0008,
	FOLLOW = 0x0010,
	PATROL2 = 0x0020,
	ALL_AIOBJ = (GUARD | AMBUSH | PATROL1 | MODIFY | FOLLOW | PATROL2)
};

typedef enum ItemStatus
{
	ITEM_ACTIVE,
	ITEM_NOT_ACTIVE,
	ITEM_DESACTIVATED,
	ITEM_INVISIBLE
};

typedef enum ItemFlags
{
	IFLAG_CLEAR_BODY = (1 << 7), // 0x0080
	IFLAG_INVISIBLE = (1 << 8),  // 0x0100
	IFLAG_REVERSE = (1 << 14),	 // 0x4000
	IFLAG_KILLED = (1 << 15),    // 0x8000
	IFLAG_ACTIVATION_MASK = 0x3E00 // bits 9-13
};

typedef struct ITEM_INFO
{
	int floor;
	DWORD touchBits;
	DWORD meshBits;
	short objectNumber;
	short currentAnimState;
	short goalAnimState;
	short requiredAnimState;
	short animNumber;
	short frameNumber;
	short roomNumber;
	short nextItem;
	short nextActive;
	short speed;
	short fallspeed;
	short hitPoints;
	unsigned short boxNumber;
	short timer;
	unsigned short flags; // ItemFlags enum
	short shade;
	short triggerFlags;
	short carriedItem;
	short afterDeath;
	short firedWeapon;
	short itemFlags[4];
	void* data;
	PHD_3DPOS pos;
	byte legacyLightData[5528];
	bool active;
	short status; // ItemStatus enum
	bool gravityStatus;
	bool hitStatus;
	bool collidable;
	bool lookedAt;
	bool dynamicLight;
	bool poisoned;
	byte aiBits; // AIObjectType enum
	bool reallyActive;
	bool inDrawRoom;
	int swapMeshFlags;
	short drawRoom;
	short TOSSPAD;
};

// used by fx->shade !
#define RGB555(r, g, b) ((r << 7) & 0x7C00 | (g << 2) & 0x3E0 | (b >> 3) & 0x1F)
#define WHITE555 RGB555(255, 255, 255)
#define GRAY555  RGB555(128, 128, 128)
#define BLACK555 RGB555(  0,   0,   0)

constexpr auto NO_ITEM = -1;
constexpr auto ALL_MESHBITS = -1;
constexpr auto NOT_TARGETABLE = -16384;
#define NUM_ITEMS 1024
#define NUM_EFFECTS 1024

void EffectNewRoom(short fxNumber, short roomNumber);
void ItemNewRoom(short itemNum, short roomNumber);
void AddActiveItem(short itemNumber);
void ClearItem(short itemNum);
short CreateItem();
void RemoveAllItemsInRoom(short roomNumber, short objectNumber);
void RemoveActiveItem(short itemNum);
void RemoveDrawnItem(short itemNum);
void InitialiseFXArray(int allocmem);
short CreateNewEffect(short roomNum);
void KillEffect(short fxNumber);
void InitialiseItem(short itemNum);
void InitialiseItemArray(int numitems);
void KillItem(short itemNum);
ITEM_INFO* find_a_fucking_item(short objectNum);
int FindItemNumber(short objectNumber);
ITEM_INFO* FindItem(short objectNumber);
