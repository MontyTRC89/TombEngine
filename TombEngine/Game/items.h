#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "Game/animation.h"
#include "Game/itemdata/itemdata.h"
#include "Math/Math.h"
#include "Specific/BitField.h"
#include "Specific/newtypes.h"

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
	Constant,
	TimedConstant
};

struct OffsetBlendData
{
	BlendType Type		 = BlendType::None;
	bool	  IsActive	 = false;
	float	  TimeActive = 0.0f;
	float	  DelayTime	 = 0.0f;

	Vector3		PosOffset	 = Vector3::Zero;
	EulerAngles OrientOffset = EulerAngles::Zero;

	float Alpha	   = 0.0f;
	float Velocity = 0.0f;
	short TurnRate = 0;
	float Time	   = 0.0f;

	void SetLinear(const Vector3& posOffset, const EulerAngles& orientOffset, float alpha, float delayInSec = 0.0f);
	void SetConstant(const Vector3& posOffset, const EulerAngles& orientOffset, float velocity, short turnRate, float delayInSec = 0.0f);
	void SetTimedConstant(const Vector3& posOffset, const EulerAngles& orientOffset, float timeInSec, float delayInSec = 0.0f);

	void Clear();
	void DisplayDebug();
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
	int BaseMesh;
	std::vector<int> MeshIndex = {};
	std::vector<BoneMutator> Mutator = {};
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
	Vector3 LightColor = Vector3::One;
	Vector3 PrimaryEffectColor = Vector3::One;
	Vector3 SecondaryEffectColor = Vector3::One;
	int Count = -1;
};

//todo we need to find good "default states" for a lot of these - squidshire 25/05/2022
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
	Vector4 Color;

	BitField TouchBits	  = BitField();
	BitField MeshBits	  = BitField();

	unsigned short Flags; // ItemFlags enum
	short ItemFlags[8];
	short TriggerFlags;

	// TODO: Move to CreatureInfo?
	uint8_t AIBits; // AIObjectType enum.
	short AfterDeath;
	short CarriedItem;

	OffsetBlendData OffsetBlend = {};

	bool TestOcb(short ocbFlags);
	void RemoveOcb(short ocbFlags);
	void ClearAllOcb();

	bool TestFlags(short id, short value);
	void SetFlags(short id, short value);

	bool TestMeshSwapFlags(unsigned int flags);
	bool TestMeshSwapFlags(const std::vector<unsigned int>& flags);
	void SetMeshSwapFlags(unsigned int flags, bool clear = false);
	void SetMeshSwapFlags(const std::vector<unsigned int>& flags, bool clear = false);

	bool IsLara() const;
	bool IsCreature() const;

	void DoOffsetBlend();

	void ResetModelToDefault();
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
void UpdateItemRoom(short itemNumber);
void UpdateAllItems();
void UpdateAllEffects();
std::vector<int> FindAllItems(short objectNumber);
ItemInfo* FindItem(int objectNumber);
int FindItem(ItemInfo* item);
void DoDamage(ItemInfo* item, int damage);
