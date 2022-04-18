#pragma once
#include "Specific/phd_global.h"

class PoseData;
struct Vector3Int;
struct ITEM_INFO;

struct ANIM_FRAME
{
	BOUNDING_BOX boundingBox;
	short offsetX;
	short offsetY;
	short offsetZ;
	std::vector<Quaternion> angles;
};

struct CHANGE_STRUCT
{
	int TargetState;
	int numberRanges;
	int rangeIndex;
};

struct RANGE_STRUCT
{
	int startFrame;
	int endFrame;
	int linkAnimNum;
	int linkFrameNum;
};

struct ANIM_STRUCT
{
	int framePtr;
	int interpolation;
	int ActiveState;
	int velocity;
	int acceleration;
	int Xvelocity;
	int Xacceleration;
	int frameBase;
	int frameEnd;
	int jumpAnimNum;
	int jumpFrameNum;
	int numberChanges;
	int changeIndex;
	int numberCommands;
	int commandIndex;
};

enum ANIMCOMMAND_TYPES
{
	COMMAND_NULL = 0,
	COMMAND_MOVE_ORIGIN,
	COMMAND_JUMP_VELOCITY,
	COMMAND_ATTACK_READY,
	COMMAND_DEACTIVATE,
	COMMAND_SOUND_FX,
	COMMAND_EFFECT
};

struct BONE_MUTATOR
{
	Vector3 Scale    = Vector3::One;
	Vector3 Offset   = Vector3::Zero;
	Vector3 Rotation = Vector3::Zero;

	bool IsEmpty() { return (Scale == Vector3::One) && (Offset == Vector3::Zero) && (Rotation == Vector3::Zero); };
};

void AnimateLara(ITEM_INFO* item);
void AnimateItem(ITEM_INFO* item);

bool HasStateDispatch(ITEM_INFO* item, int targetState = -1);
bool TestLastFrame(ITEM_INFO* item, int animNumber = -1);

void TranslateItem(ITEM_INFO* item, int x, int y, int z);
void SetAnimation(ITEM_INFO* item, int animIndex, int frameToStart = 0);

int GetCurrentRelativeFrameNumber(ITEM_INFO* item);
int GetFrameNumber(ITEM_INFO* item, int frameToStart);
int GetFrameNumber(int objectID, int animNumber, int frameToStart);
int GetFrameCount(int animNumber);
int GetNextAnimState(ITEM_INFO* item);
int GetNextAnimState(int objectID, int animNumber);
bool GetChange(ITEM_INFO* item, ANIM_STRUCT* anim);
int GetFrame(ITEM_INFO* item, ANIM_FRAME* framePtr[], int* rate);
ANIM_FRAME* GetBestFrame(ITEM_INFO* item);

BOUNDING_BOX* GetBoundsAccurate(ITEM_INFO* item);
void GetLaraJointPosition(Vector3Int* pos, int laraMeshIndex);
void GetJointAbsPosition(ITEM_INFO* item, Vector3Int* vec, int joint);

void ClampRotation(PoseData* pose, float angle, float rotation);

void DrawAnimatingItem(ITEM_INFO* item);
