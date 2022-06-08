#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "Game/animation.h"
#include "Game/itemdata/itemdata.h"
#include "Specific/newtypes.h"
#include "Specific/phd_global.h"

enum GAME_OBJECT_ID : short;

// used by fx->shade !
#define RGB555(r, g, b) ((r << 7) & 0x7C00 | (g << 2) & 0x3E0 | (b >> 3) & 0x1F)
#define WHITE555 RGB555(255, 255, 255)
#define GRAY555  RGB555(128, 128, 128)
#define BLACK555 RGB555(  0,   0,   0)

constexpr auto NO_ITEM = -1;
constexpr auto NOT_TARGETABLE = -16384;
constexpr auto NUM_ITEMS = 1024;

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

constexpr unsigned __int64 ALL_JOINT_BITS = INT64_MAX;
constexpr unsigned __int64 NO_JOINT_BITS = 0;

enum class JointBitType
{
	Touch,
	Mesh,
	Meshswap
};

struct EntityAnimationData
{
	int AnimNumber;
	int FrameNumber;
	int ActiveState;
	int TargetState;
	int RequiredState; // TODO: Phase out this weird feature.

	bool Airborne;
	int Velocity;
	int VerticalVelocity;
	int LateralVelocity;
	std::vector<BONE_MUTATOR> Mutator;
};

//todo we need to find good "default states" for a lot of these - squidshire 25/05/2022
struct ItemInfo
{
	GAME_OBJECT_ID ObjectNumber;
	int Status;	// ItemStatus enum.
	bool Active;
	short NextItem;
	short NextActive;

	ITEM_DATA Data;
	EntityAnimationData Animation;
	PHD_3DPOS Pose;
	PHD_3DPOS StartPose;
	int Floor;

	int HitPoints;
	bool HitStatus;
	bool LookedAt;
	bool Collidable;
	bool InDrawRoom;

	ROOM_VECTOR Location;
	short RoomNumber;
	int BoxNumber;
	int Timer;
	short Shade;

	unsigned __int64 TouchBits;
	unsigned __int64 MeshBits;
	unsigned __int64 SwapMeshBits;

	unsigned short Flags; // ItemFlags enum
	short ItemFlags[8];
	short TriggerFlags;

	// TODO: Move to CreatureInfo?
	uint8_t AIBits; // AIObjectType enum.
	short AfterDeath;
	short CarriedItem;

	// Lua
	std::string LuaName;
	std::string LuaCallbackOnKilledName;
	std::string LuaCallbackOnHitName;
	std::string LuaCallbackOnCollidedWithObjectName;
	std::string LuaCallbackOnCollidedWithRoomName;

	void SetBits(std::vector<int> jointIndices, JointBitType testType);
	void ClearBits(std::vector<int> jointIndices, JointBitType testType);
	bool TestBits(std::vector<int> jointIndices, JointBitType testType);
	void SetBits(int jointIndex, JointBitType testType);
	void ClearBits(int jointIndex, JointBitType testType);
	bool TestBits(int jointIndex, JointBitType testType);
};

void EffectNewRoom(short fxNumber, short roomNumber);
void ItemNewRoom(short itemNumber, short roomNumber);
void AddActiveItem(short itemNumber);
void ClearItem(short itemNumber);
short CreateItem();
void RemoveAllItemsInRoom(short roomNumber, short objectNumber);
void RemoveActiveItem(short itemNumber);
void RemoveDrawnItem(short itemNumber);
void InitialiseFXArray(int allocateMemory);
short CreateNewEffect(short roomNumber);
void KillEffect(short fxNumber);
void InitialiseItem(short itemNumber);
void InitialiseItemArray(int totalItems);
void KillItem(short itemNumber);
void UpdateItemRoom(ItemInfo* item, int height, int xOffset = 0, int zOffset = 0);
std::vector<int> FindAllItems(short objectNumber);
ItemInfo* FindItem(int objectNumber);
int FindItem(ItemInfo* item);
