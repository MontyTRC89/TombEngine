#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "Game/animation.h"
#include "Game/itemdata/itemdata.h"
#include "Math/Math.h"
#include "Specific/newtypes.h"
#include "Specific/BitField.h"

using namespace TEN::Utils;

enum GAME_OBJECT_ID : short;

constexpr auto NO_ITEM = -1;
constexpr auto NOT_TARGETABLE = -16384;
constexpr auto NUM_ITEMS = 1024;

constexpr unsigned int ALL_JOINT_BITS = UINT_MAX;
constexpr unsigned int NO_JOINT_BITS  = 0;

enum AIObjectType
{
	NO_AI	  = 0,
	GUARD	  = (1 << 0),
	AMBUSH	  = (1 << 1),
	PATROL1	  = (1 << 2),
	MODIFY	  = (1 << 3),
	FOLLOW	  = (1 << 4),
	PATROL2	  = (1 << 5),
	ALL_AIOBJ = (GUARD | AMBUSH | PATROL1 | MODIFY | FOLLOW | PATROL2)
};

enum ItemStatus
{
	ITEM_NOT_ACTIVE	 = 0,
	ITEM_ACTIVE		 = 1,
	ITEM_DEACTIVATED = 2,
	ITEM_INVISIBLE	 = 3
};

enum ItemFlags
{
	IFLAG_CLEAR_BODY	  = (1 << 7),
	IFLAG_INVISIBLE		  = (1 << 8),
	IFLAG_REVERSE		  = (1 << 14),
	IFLAG_KILLED		  = (1 << 15),
	IFLAG_ACTIVATION_MASK = 0x3E00 // bits 9-13
};

enum class BlendType
{
	None,
	Linear,
	Constant
};

struct OffsetBlendData
{
	bool  IsActive	 = false;
	float Delay		 = 0.0f;

	BlendType	Type		= BlendType::None;
	Vector3i	Position	= Vector3i::Zero;
	EulerAngles Orientation = EulerAngles::Zero;

	// Linear type
	float Alpha = 0.0f;

	// Constant type
	float Velocity = 0.0f;
	short TurnRate = 0;
};

struct EntityAnimationData
{
	int AnimNumber	  = -1;
	int FrameNumber	  = -1;
	int ActiveState	  = -1;
	int TargetState	  = -1;
	int RequiredState = -1; // TODO: Phase out this weird feature.

	bool IsAirborne	= false;
	Vector3 Velocity = Vector3::Zero; // CONVENTION: +X = right, +Y = down, +Z = forward
	std::vector<BoneMutator> Mutator = {};
};

//todo we need to find good "default states" for a lot of these - squidshire 25/05/2022
struct ItemInfo
{
	GAME_OBJECT_ID ObjectNumber;

	int Status;	// ItemStatus enum.
	bool Active;

	short Index;
	short NextItem;
	short NextActive;

	ITEM_DATA Data;
	EntityAnimationData Animation;
	Pose StartPose;
	Pose Pose;
	ROOM_VECTOR Location;
	short RoomNumber;
	int Floor;

	int HitPoints;
	bool HitStatus;
	bool LookedAt;
	bool Collidable;
	bool InDrawRoom;

	int BoxNumber;
	int Timer;
	Vector4 Color;

	BitField TouchBits	  = BitField();
	BitField MeshBits	  = BitField();
	BitField MeshSwapBits = BitField();

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

	OffsetBlendData OffsetBlend = {};

	bool TestOcb(short ocbFlags);
	void RemoveOcb(short ocbFlags);
	void ClearAllOcb();

	bool TestFlags(short id, short value);
	void SetFlags(short id, short value);

	bool IsLara();
	bool IsCreature();

	void SetOffsetBlend(const Vector3i& pos, const EulerAngles& orient, float alpha, float delay = 0.0f);
	void SetOffsetBlend(const Vector3i& pos, const EulerAngles& orient, float velocity, short turnRate, float delay = 0.0f);
	void ClearOffsetBlend();
	void DoOffsetBlend();
};

bool TestState(int refState, const std::vector<int>& stateList);
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
void DoDamage(ItemInfo* item, int damage);
