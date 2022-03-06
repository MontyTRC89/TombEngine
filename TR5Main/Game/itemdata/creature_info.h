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

struct LOT_INFO 
{
	std::vector<BOX_NODE> node;
	int head;
	int tail;
	int searchNumber;
	int blockMask;
	short step;
	short drop;
	short zoneCount;
	int targetBox;
	int requiredBox;
	short fly;
	bool canJump;
	bool canMonkey;
	bool isAmphibious;
	bool isJumping;
	bool isMonkeying;
	PHD_VECTOR target;
	ZoneType zone;
	bool initialised;
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

struct CREATURE_INFO 
{
	short jointRotation[4];
	short maximumTurn;
	short flags;
	bool alerted;
	bool headLeft;
	bool headRight;
	bool reachedGoal;
	bool hurtByLara;
	bool patrol2;
	bool jumpAhead;
	bool monkeyAhead;
	MoodType mood;
	ITEM_INFO* enemy;
	short aiTargetNum;
	ITEM_INFO* aiTarget;
	short pad;
	short itemNum;
	PHD_VECTOR target;
	LOT_INFO LOT;
#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	CreatureAIPriority priority;
	size_t framesSinceLOTUpdate;
#endif
};
