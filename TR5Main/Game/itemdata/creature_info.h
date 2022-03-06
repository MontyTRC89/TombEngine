#pragma once
#include <vector>
#include "Specific/phd_global.h"

struct ITEM_INFO;

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

	bool Initialised;
	bool CanJump;
	bool CanMonkey;
	bool IsAmphibious;
	bool IsJumping;
	bool IsMonkeying;

	PHD_VECTOR Target;
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
	bool ReachedGoal;
	bool JumpAhead;
	bool MonkeySwingAhead;
	bool Friendly;
	bool Poisoned;
	bool HurtByLara;

	short Tosspad;
	short LocationAI;
	short FiredWeapon;

	MoodType Mood;
	ITEM_INFO* Enemy;
	short AITargetNumber;
	ITEM_INFO* AITarget;
	short Pad;				// Unused?
	PHD_VECTOR Target;
	LOTInfo LOT;

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	CreatureAIPriority Priority;
	size_t FramesSinceLOTUpdate;
#endif

	short Flags;
};
