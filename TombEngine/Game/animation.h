#pragma once
#include "Specific/phd_global.h"

struct PHD_3DPOS;
struct Vector3Int;
struct ItemInfo;

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
	int Interpolation;
	int ActiveState;

	// CONVENTION: +X is right, +Y is down, +Z is forward.
	Vector3 VelocityStart = Vector3::Zero;
	Vector3 VelocityEnd	  = Vector3::Zero;

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

struct BoneMutator
{
	Vector3 Offset   = Vector3::Zero;
	Vector3 Rotation = Vector3::Zero;
	Vector3 Scale    = Vector3::One;

	bool IsEmpty() { return (Offset == Vector3::Zero) && (Rotation == Vector3::Zero) && (Scale == Vector3::One); };
};

void AnimateLara(ItemInfo* item);
void AnimateItem(ItemInfo* item);

bool HasStateDispatch(ItemInfo* item, int targetState = -1);
bool TestLastFrame(ItemInfo* item, int animNumber = -1);

void TranslateItem(ItemInfo* item, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
void TranslateItem(ItemInfo* item, Vector3Shrt orient, float distance);
void TranslateItem(ItemInfo* item, Vector3 direction, float distance);
void SetAnimation(ItemInfo* item, int animIndex, int frameToStart = 0);

int GetCurrentRelativeFrameNumber(ItemInfo* item);
int GetFrameNumber(ItemInfo* item, int frameToStart);
int GetFrameNumber(int objectID, int animNumber, int frameToStart);
int GetFrameCount(int animNumber);
int GetNextAnimState(ItemInfo* item);
int GetNextAnimState(int objectID, int animNumber);
bool GetChange(ItemInfo* item, ANIM_STRUCT* anim);
int GetFrame(ItemInfo* item, ANIM_FRAME* framePtr[], int* rate);
ANIM_FRAME* GetBestFrame(ItemInfo* item);
BOUNDING_BOX* GetBoundsAccurate(ItemInfo* item);

void ClampRotation(PHD_3DPOS* pose, short angle, short rotation); 
void DrawAnimatingItem(ItemInfo* item);

Vector3Int GetLaraJointPosition(int jointIndex, Vector3Int offset = Vector3Int::Zero);
void GetLaraJointPosition(Vector3Int* offset, int jointIndex);
Vector3Int GetJointAbsPosition(ItemInfo* item, int jointIndex, Vector3Int offset = Vector3Int::Zero);
void GetJointAbsPosition(ItemInfo* item, Vector3Int* vec, int jointIndex);
