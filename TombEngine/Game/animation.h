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

void AnimateLara(ItemInfo* item);
void AnimateItem(ItemInfo* item);

bool HasStateDispatch(ItemInfo* item, int targetState = -1);
bool TestLastFrame(ItemInfo* item, int animNumber = -1);

void TranslateItem(ItemInfo* item, short angle, float forward, float vertical = 0.0f, float lateral = 0.0f);
void TranslateItem(ItemInfo* item, Vector3Shrt orient, float distance);
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
void GetLaraJointPosition(Vector3Int* pos, int laraMeshIndex);
void GetJointAbsPosition(ItemInfo* item, Vector3Int* vec, int joint);

void ClampRotation(PHD_3DPOS* pos, short angle, short rotation); 

void DrawAnimatingItem(ItemInfo* item);
