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
	ZONE_SKELLY,
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
	ZONE_SOPHIALEE, // dont want sophia to go down again.
	ZONE_APE,       // only half block climb
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

	bool CanJump = false;
	bool CanMonkey = false;
	bool IsJumping = false;
	bool IsMonkeying = false;
	bool IsAmphibious = false;

	Vector3Int Target = Vector3Int::Zero;
	ZoneType Zone = ZoneType::ZONE_NULL;
};

enum class MoodType 
{
	None,
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
	short ItemNumber = -1;

	LOTInfo	   LOT			  = {};
	MoodType   Mood			  = MoodType::None;
	ItemInfo*  Enemy		  = nullptr;
	ItemInfo*  AITarget		  = nullptr;
	short	   AITargetNumber = -1;
	Vector3Int Target		  = Vector3Int::Zero;

	short MaxTurn = 0;
	short JointRotation[4] = {};
	bool HeadLeft = false;
	bool HeadRight = false;

	bool Patrol			  = false; // Unused?
	bool Alerted		  = false;
	bool Friendly		  = false;
	bool HurtByLara		  = false;
	bool Poisoned		  = false;
	bool JumpAhead		  = false;
	bool MonkeySwingAhead = false;
	bool ReachedGoal	  = false;

	short FiredWeapon;
	short Tosspad;
	short LocationAI;
	short Flags = 0;

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	CreatureAIPriority Priority = CreatureAIPriority::None;
	size_t FramesSinceLOTUpdate = 0;
#endif
};
