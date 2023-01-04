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
constexpr auto NUM_ITEMFLAGS_COUNT = 8;

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
	IFLAG_TRIGGERED       = (1 << 5),
	IFLAG_CLEAR_BODY	  = (1 << 7),
	IFLAG_INVISIBLE		  = (1 << 8),
	IFLAG_REVERSE		  = (1 << 14),
	IFLAG_KILLED		  = (1 << 15),
	IFLAG_ACTIVATION_MASK = 0x3E00 // bits 9-13 (IFLAG_CODEBITS)
};

constexpr unsigned int ALL_JOINT_BITS = UINT_MAX;
constexpr unsigned int NO_JOINT_BITS  = 0;

enum class JointBitType
{
	Touch,
	Mesh,
	MeshSwap
};

enum class EffectType
{
	None,
	Fire,
	Sparks,
	Smoke,
	ElectricIgnite,
	RedIgnite,
	Custom
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
};

struct EntityModelData
{
	int BaseMesh = 0;

	Vector4 Color = Vector4::Zero;

	std::vector<int>		 MeshIndex = {};
	std::vector<BoneMutator> Mutator   = {};
};

struct EntityCallbackData
{
	std::string OnKilled;
	std::string OnHit;
	std::string OnObjectCollided;
	std::string OnRoomCollided;
};

struct EntityEffectData
{
	EffectType Type = EffectType::None;
	Vector3 LightColor = Vector3::Zero;
	Vector3 PrimaryEffectColor = Vector3::Zero;
	Vector3 SecondaryEffectColor = Vector3::Zero;
	int Count = -1;
};

// TODO: we need to find good "default states" for a lot of these - squidshire 25/05/2022

struct ItemInfo
{
	GAME_OBJECT_ID ObjectNumber;
	std::string Name;

	int Status;	// ItemStatus enum.
	bool Active;

	short Index;
	short NextItem;
	short NextActive;

	ITEM_DATA Data;
	EntityAnimationData Animation;
	EntityCallbackData Callbacks;
	EntityModelData Model;
	EntityEffectData Effect;
	
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

	BitField TouchBits	  = BitField();
	BitField MeshBits	  = BitField();

	unsigned short Flags; // ItemFlags enum
	short ItemFlags[NUM_ITEMFLAGS_COUNT];
	short TriggerFlags;

	// TODO: Move to CreatureInfo?
	uint8_t AIBits; // AIObjectType enum.
	short AfterDeath;
	short CarriedItem;

	bool TestOcb(short ocbFlags) const;
	void RemoveOcb(short ocbFlags);
	void ClearAllOcb();

	// test value (ItemFlags[id] & value)
	bool TestFlag(short id, short value) const;
	// test value (ItemFlags[id] == value)
	bool TestFlagEqual(short id, short value) const;
	// test value (ItemFlags[id] != value)
	bool TestFlagDiff(short id, short value) const;
	// set value (ItemFlags[id] = value)
	void SetFlag(short id, short value);
	// get value (return ItemFlags[id])
	short GetFlag(short id);
	// remove value (ItemFlags[id] &= ~value)
	void RemoveFlag(short id, short value);

	bool TestMeshSwapFlags(unsigned int flags);
	bool TestMeshSwapFlags(const std::vector<unsigned int>& flags);
	void SetMeshSwapFlags(unsigned int flags, bool clear = false);
	void SetMeshSwapFlags(const std::vector<unsigned int>& flags, bool clear = false);

	bool IsLara() const;
	bool IsCreature() const;

	void ResetModelToDefault();
};

bool TestState(int refState, const std::vector<int>& stateList);
void EffectNewRoom(short fxNumber, short roomNumber);
void ItemNewRoom(short itemNumber, short roomNumber);
void AddActiveItem(short itemNumber);
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
bool UpdateItemRoom(short itemNumber);
void UpdateAllItems();
void UpdateAllEffects();
std::vector<int> FindAllItems(short objectNumber);
ItemInfo* FindItem(int objectNumber);
int FindItem(ItemInfo* item);
void DoDamage(ItemInfo* item, int damage);
