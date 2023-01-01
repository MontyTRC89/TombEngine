#pragma once
#include "Math/Math.h"

struct ItemInfo;

enum class ZoneType
{
	// BASIC ZONES (used by LOT.zone and loaded from g_Level.Zones[ZoneType])

	// Enable jump (also 1 block vault/fall)
	Skeleton,
	// Enable 1 click vault/fall (default)
	Basic,
	// Only underwater (exception: crocodile can go on land)
	// TODO: underwater creature can go on land like the crocodile which is wrong since the flag IsAmphibious was not set for them !
	Water,
	// Enable 1 block vault/fall
	Human,
	// Can fly anywhere except in water room
	Flyer,
	MaxZone,

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

struct BoxNode
{
	int exitBox		  = 0;
	int searchNumber  = 0;
	int nextExpansion = 0;
	int boxNumber	  = 0;
};

struct LOTInfo 
{
	bool Initialised = false;

	std::vector<BoxNode> Node = {};
	int Head = 0;
	int Tail = 0;

	ZoneType Zone	= ZoneType::Basic;
	Vector3i Target = Vector3i::Zero;

	int	  TargetBox	   = 0;
	int	  RequiredBox  = 0;
	int	  SearchNumber = 0;
	int	  BlockMask	   = 0;
	short ZoneCount	   = 0;
	short Step		   = 0;
	short Drop		   = 0;
	short Fly		   = 0;

	bool IsAmphibious = false;
	bool IsJumping	  = false;
	bool IsMonkeying  = false;

	bool CanJump	  = false;
	bool CanMonkey	  = false;
};

struct CreatureInfo 
{
	int ItemNumber = -1;

	LOTInfo	  LOT			 = {};
	MoodType  Mood			 = MoodType::Bored;
	ItemInfo* Enemy			 = nullptr;
	ItemInfo* AITarget		 = nullptr;
	int		  AITargetNumber = -1;
	Vector3i  Target		 = Vector3i::Zero;

	short MaxTurn		   = 0;
	short JointRotation[4] = {};
	bool  HeadLeft		   = false;
	bool  HeadRight		   = false;

	bool Patrol			  = false; // Unused?
	bool Alerted		  = false;
	bool Friendly		  = false;
	bool HurtByLara		  = false;
	bool Poisoned		  = false;
	bool JumpAhead		  = false;
	bool MonkeySwingAhead = false;
	bool ReachedGoal	  = false;

	short FiredWeapon = 0;
	short Tosspad	  = 0;
	short LocationAI  = 0;
	short Flags		  = 0;

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	CreatureAIPriority Priority = CreatureAIPriority::None;
	size_t FramesSinceLOTUpdate = 0;
#endif
};
