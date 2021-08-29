#pragma once
#include "phd_global.h"
#include <vector>
struct BOX_NODE {
	int exitBox;
	int searchNumber;
	int nextExpansion;
	int boxNumber;
};

enum ZoneType : char {
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

struct LOT_INFO {
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
};
enum MOOD_TYPE {
	BORED_MOOD,
	ATTACK_MOOD,
	ESCAPE_MOOD,
	STALK_MOOD
};
struct CREATURE_INFO {
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
	MOOD_TYPE mood;
	ITEM_INFO* enemy;
	ITEM_INFO aiTarget;
	short pad;
	short itemNum;
	PHD_VECTOR target;
	LOT_INFO LOT;
};