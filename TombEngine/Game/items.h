#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "Game/animation.h"
#include "Game/itemdata/itemdata.h"
#include "Objects/game_object_ids.h"
#include "Math/Math.h"
#include "Specific/newtypes.h"
#include "Specific/BitField.h"

using namespace TEN::Utils;

constexpr auto NO_ITEM		  = -1;
constexpr auto NOT_TARGETABLE = -16384;

constexpr auto NUM_ITEMS	 = 1024;
constexpr auto NUM_ITEM_FLAGS = 8;

constexpr unsigned int ALL_JOINT_BITS = UINT_MAX;
constexpr unsigned int NO_JOINT_BITS = 0;
constexpr int		   NO_JOINT = -1;

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
	IFLAG_ACTIVATION_MASK = (0x1F << 9), // Bits 9-13 (IFLAG_CODEBITS)
	IFLAG_REVERSE		  = (1 << 14),
	IFLAG_KILLED		  = (1 << 15)
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
	GAME_OBJECT_ID AnimObjectID = ID_NO_OBJECT;

	int AnimNumber	  = 0; // g_Level.Anims index.
	int FrameNumber	  = 0; // g_Level.Frames index.
	int ActiveState	  = 0;
	int TargetState	  = 0;
	int RequiredState = NO_STATE;

	bool	IsAirborne = false;
	Vector3 Velocity   = Vector3::Zero; // CONVENTION: +X = Right, +Y = Down, +Z = Forward
};

struct EntityModelData
{
	int BaseMesh = 0;

	Vector4 Color = Vector4::Zero;

	std::vector<int>		 MeshIndex = {};
	std::vector<BoneMutator> Mutators  = {};
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
	EffectType Type					= EffectType::None;
	Vector3	   LightColor			= Vector3::Zero;
	Vector3	   PrimaryEffectColor	= Vector3::Zero;
	Vector3	   SecondaryEffectColor = Vector3::Zero;
	int		   Count				= -1;
};

// TODO: We need to find good "default states" for a lot of these. -- squidshire 25/05/2022
struct ItemInfo
{
	GAME_OBJECT_ID ObjectNumber = ID_NO_OBJECT; // ObjectID
	std::string	   Name			= {};

	int Status;	// ItemStatus enum.
	bool Active;

	short Index;
	short NextItem;
	short NextActive;

	ItemData Data;
	EntityAnimationData Animation;
	EntityCallbackData Callbacks;
	EntityModelData Model;
	EntityEffectData Effect;
	
	Pose StartPose;
	Pose Pose;
	RoomVector Location;
	short RoomNumber;
	int Floor;

	int HitPoints;
	bool HitStatus;
	bool LookedAt;
	bool Collidable;
	bool InDrawRoom;

	int BoxNumber;
	int Timer;

	BitField TouchBits = BitField::Default;
	BitField MeshBits  = BitField::Default;

	unsigned short Flags; // ItemFlags enum
	short ItemFlags[NUM_ITEM_FLAGS];
	short TriggerFlags;

	// TODO: Move to CreatureInfo?
	unsigned char AIBits; // AIObjectType enum.
	short AfterDeath;
	short CarriedItem;

	bool TestOcb(short ocbFlags) const;
	void RemoveOcb(short ocbFlags);
	void ClearAllOcb();
	
	bool  TestFlags(int id, short flags) const;		// ItemFlags[id] & flags
	bool  TestFlagField(int id, short flags) const; // ItemFlags[id] == flags
	short GetFlagField(int id) const;				// ItemFlags[id]
	void  SetFlagField(int id, short flags);		// ItemFlags[id] = flags
	void  ClearFlags(int id, short flags);			// ItemFlags[id] &= ~flags

	bool TestMeshSwapFlags(unsigned int flags);
	bool TestMeshSwapFlags(const std::vector<unsigned int>& flags);
	void SetMeshSwapFlags(unsigned int flags, bool clear = false);
	void SetMeshSwapFlags(const std::vector<unsigned int>& flags, bool clear = false);

	bool IsLara() const;
	bool IsCreature() const;
	bool IsBridge() const;

	void ResetModelToDefault();
};

bool TestState(int refState, const std::vector<int>& stateList);
void EffectNewRoom(short fxNumber, short roomNumber);
void ItemNewRoom(short itemNumber, short roomNumber);
void AddActiveItem(short itemNumber);
short CreateItem();
void RemoveAllItemsInRoom(short roomNumber, short objectNumber);
void RemoveActiveItem(short itemNumber, bool killed = true);
void RemoveDrawnItem(short itemNumber);
void InitializeFXArray();
short CreateNewEffect(short roomNumber);
void KillEffect(short fxNumber);
void InitializeItem(short itemNumber);
void InitializeItemArray(int totalItems);
void KillItem(short itemNumber);
bool UpdateItemRoom(short itemNumber);
void UpdateAllItems();
void UpdateAllEffects();
const std::string& GetObjectName(GAME_OBJECT_ID objectID);
std::vector<int> FindAllItems(GAME_OBJECT_ID objectID);
std::vector<int> FindCreatedItems(GAME_OBJECT_ID objectID);
ItemInfo* FindItem(GAME_OBJECT_ID objectID);
int FindItem(ItemInfo* item);
void DoDamage(ItemInfo* item, int damage);
void DoItemHit(ItemInfo* target, int damage, bool isExplosive, bool allowBurn = true);
void DefaultItemHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
short SpawnItem(ItemInfo* item, GAME_OBJECT_ID objectNumber);

Vector3i GetNearestSectorCenter(const Vector3i& pos);
