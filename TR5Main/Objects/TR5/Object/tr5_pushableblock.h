#pragma once
#include "items.h"
#include "collide.h"

struct PUSHABLE_INFO
{
	int height;				// height for collision, also in floor procedure
	int weight;
	int stackLimit;
	int moveX;				// used for pushable movement code
	int moveZ;				// used for pushable movement code
	short linkedIndex;		// using itemFlags[1] for now
	short gravity;			// fall acceleration
	short loopSound;		// looped sound index for movement
	short stopSound;		// ending sound index
	short fallSound;		// sound on hitting floor (if dropped)
	short climb;			// not used for now
	bool canFall;			// OCB 32
	bool hasFloorCeiling;			// has floor and ceiling procedures (OCB 64)
	bool disablePull;		// OCB 128
	bool disablePush;		// OCB 256
	bool disableW;			// OCB 512 (W+E)
	bool disableE;			// OCB 512 (W+E)
	bool disableN;			// OCB 1024 (N+S)
	bool disableS;			// OCB 1024 (N+S)
};

void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber);
void InitialisePushableBlock(short itemNum);
void PushableBlockControl(short itemNumber);
void PushableBlockCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
int TestBlockMovable(ITEM_INFO* item, int blokhite);
int TestBlockPush(ITEM_INFO* item, int blockhite, unsigned short quadrant);
int TestBlockPull(ITEM_INFO* item, int blockhite, short quadrant);
void MoveStackXZ(short itemNum);
void MoveStackY(short itemNum, int y);
void RemoveBridgeStack(short itemNum);
void AddBridgeStack(short itemNum);
void RemoveFromStack(short itemNum);
int FindStack(short itemNum);
int GetStackHeight(ITEM_INFO* item);
bool CheckStackLimit(ITEM_INFO* item);
void PushLoop(ITEM_INFO* item);
void PushEnd(ITEM_INFO* item);
std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z);
std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z);
int PushableBlockFloorBorder(short itemNumber);
int PushableBlockCeilingBorder(short itemNumber);
