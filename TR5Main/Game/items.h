#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Game/animation.h"
#include "Game/itemdata/itemdata.h"
#include "Specific/newtypes.h"
#include "Specific/phd_global.h"

enum GAME_OBJECT_ID : short;

constexpr unsigned int NO_MESH_BITS = UINT_MAX;

enum AIObjectType
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

enum ItemStatus
{
	ITEM_NOT_ACTIVE = 0,
	ITEM_ACTIVE = 1,
	ITEM_DEACTIVATED = 2,
	ITEM_INVISIBLE = 3
};

enum ItemFlags
{
	IFLAG_CLEAR_BODY = (1 << 7), // 0x0080
	IFLAG_INVISIBLE = (1 << 8),  // 0x0100
	IFLAG_REVERSE = (1 << 14),	 // 0x4000
	IFLAG_KILLED = (1 << 15),    // 0x8000
	IFLAG_ACTIVATION_MASK = 0x3E00 // bits 9-13
};

struct ITEM_INFO
{
	std::string LuaName;
	GAME_OBJECT_ID ObjectNumber;
	int BoxNumber;
	ITEM_DATA Data;
	bool Active;
	short NextItem;
	short NextActive;

	PHD_3DPOS StartPosition;
	PHD_3DPOS Position;
	ROOM_VECTOR Location;
	short RoomNumber;
	int Floor;
	std::vector<BONE_MUTATOR> Mutator;

	int ActiveState;
	int TargetState;
	int RequiredState; // TODO: Phase out this weird feature.
	int AnimNumber;
	int FrameNumber;

	short Velocity;
	short VerticalVelocity;
	bool Airborne;

	short HitPoints;
	short Timer;
	short Shade;

	uint32_t MeshBits;

	short ItemFlags[8];
	short TriggerFlags;
	uint16_t Flags; // ItemFlags enum
	uint32_t SwapMeshFlags;

	short Status; // ItemStatus enum
	bool HitStatus;
	bool Collidable;
	bool LookedAt;
	bool InDrawRoom;

	short DrawRoom;	// Unused?

	// TODO: Move to CreatureInfo?
	uint32_t TouchBits;
	uint8_t AIBits; // AIObjectType enum.
	short AfterDeath;
	short CarriedItem;
};

// used by fx->shade !
#define RGB555(r, g, b) ((r << 7) & 0x7C00 | (g << 2) & 0x3E0 | (b >> 3) & 0x1F)
#define WHITE555 RGB555(255, 255, 255)
#define GRAY555  RGB555(128, 128, 128)
#define BLACK555 RGB555(  0,   0,   0)

constexpr auto NO_ITEM = -1;
constexpr auto ALL_MESHBITS = -1;
constexpr auto NOT_TARGETABLE = -16384;
constexpr auto NUM_ITEMS = 1024;

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
void InitialiseItemArray(int numItems);
void KillItem(short itemNum);
void UpdateItemRoom(ITEM_INFO* item, int height, int xOffset = 0, int zOffset = 0);
std::vector<int> FindAllItems(short objectNumber);
ITEM_INFO* FindItem(int object_number);
int FindItem(ITEM_INFO* item);
