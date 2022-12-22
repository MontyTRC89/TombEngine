#pragma once
#include "Math/Math.h"

using std::vector;

struct ItemInfo;

struct BOX_NODE 
{
	int exitBox;
	int searchNumber;
	int nextExpansion;
	int boxNumber;
};

enum class ZoneType
{
	// BASIC ZONES (used by LOT.zone and loaded from g_Level.Zones[ZoneType])
	
	// Can jump
	Skeleton,
	Basic,

	// Only underwater (exception: crocodile can go on land)
	Water,

	// Enable jump and monkey
	Human,
	Flyer,

	// ADDED ZONES (above zones are used for LOT.zone):

	// Enable jump (also 1 block vault/fall)
	HumanJump,

	// Enable jump and monkey swing (also 1 block climb/fall)
	HumanJumpAndMonkey,
	Spider,
	Blockable, // For large creatures such as trex and shiva.
	SophiaLee, // Prevents Sophia from going to lower levels again.
	Ape,	   // Only 1/2 block vault.
	HumanLongJumpAndMonkey,
};

struct LOTInfo
{
	bool Initialised;

	vector<BOX_NODE> Node = {};
	int Head;
	int Tail;

	ZoneType Zone = ZoneType::Basic;
	Vector3i Target = Vector3i::Zero;
	int SearchNumber;
	int BlockMask;
	short Step;
	short Drop;
	short ZoneCount;
	int TargetBox;
	int RequiredBox;
	short Fly;

	bool CanJump	  = false;
	bool CanMonkey	  = false;
	bool IsJumping	  = false;
	bool IsMonkeying  = false;
	bool IsAmphibious = false;
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
	Low,
	Medium,
	High
};

struct CreatureInfo 
{
	short ItemNumber = -1;

	LOTInfo	  LOT			 = {};
	MoodType  Mood			 = MoodType::Bored;
	ItemInfo* Enemy			 = nullptr;
	ItemInfo* AITarget		 = nullptr;
	short	  AITargetNumber = -1;
	Vector3i  Target		 = Vector3i::Zero;

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
