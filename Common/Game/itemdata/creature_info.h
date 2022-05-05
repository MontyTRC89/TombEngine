#pragma once
#include <vector>
#include "Specific/phd_global.h"

struct ItemInfo;

struct BOX_NODE 
{
	int exitBox;
	int searchNumber;
	int nextExpansion;
	int boxNumber;
};

enum ZoneType : char 
{
	ZONE_NULL = -1,  // default zone
	ZONE_SKELLY = 0,
	ZONE_BASIC,
	ZONE_FLYER,
	ZONE_HUMAN_CLASSIC,
	ZONE_VON_CROY,
	ZONE_WATER,
	ZONE_MAX,
	/// custom zone (using zone above for LOT.zone):
	ZONE_HUMAN_JUMP_AND_MONKEY,
	ZONE_HUMAN_JUMP,
	ZONE_SPIDER,
	ZONE_BLOCKABLE, // for trex, shiva, etc..
	ZONE_SOPHIALEE, // dont want sophia to go down again !
	ZONE_APE,       // only 2 click climb
	ZONE_HUMAN_LONGJUMP_AND_MONKEY,
};

struct LOTInfo 
{
	bool Initialised;

	std::vector<BOX_NODE> Node;
	int Head;
	int Tail;

	int SearchNumber;
	int BlockMask;
	short Step;
	short Drop;
	short ZoneCount;
	int TargetBox;
	int RequiredBox;
	short Fly;

	bool CanJump;
	bool CanMonkey;
	bool IsJumping;
	bool IsMonkeying;
	bool IsAmphibious;

	Vector3Int Target;
	ZoneType Zone;
};

enum class MoodType 
{
	Bored,
	Attack,
	Escape,
	Stalk
};

enum class CreatureAIPriority
{
	None,
	High,
	Medium,
	Low
};

struct CreatureInfo 
{
	short ItemNumber;

	short MaxTurn;
	short JointRotation[4];
	bool HeadLeft;
	bool HeadRight;

	bool Patrol;			// Unused?
	bool Alerted;
	bool Friendly;
	bool HurtByLara;
	bool Poisoned;
	bool JumpAhead;
	bool MonkeySwingAhead;
	bool ReachedGoal;

	short Tosspad;
	short LocationAI;
	short FiredWeapon;

	LOTInfo LOT;
	MoodType Mood;
	ItemInfo* Enemy;
	short AITargetNumber;
	ItemInfo* AITarget;
	short Pad;				// Unused?
	Vector3Int Target;

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	CreatureAIPriority Priority;
	size_t FramesSinceLOTUpdate;
#endif

	short Flags;
};
