#pragma once
#include "Specific/phd_global.h"

struct PHD_3DPOS;
struct PHD_VECTOR;
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
	int goalAnimState;
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
	int currentAnimState;
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

void AnimateItem(ITEM_INFO* item);
void TranslateItem(ITEM_INFO* item, int x, int y, int z);
void SetAnimation(ITEM_INFO* item, int animIndex, int frameToStart = 0);
int GetFrameNumber(ITEM_INFO* item, int frameToStart);
int GetFrameNumber(int objectID, int animNumber, int frameToStart);
int GetFrameCount(int animNumber);
int GetNextAnimState(ITEM_INFO* item);
int GetNextAnimState(int objectID, int animNumber);
bool GetChange(ITEM_INFO* item, ANIM_STRUCT* anim);
int GetFrame(ITEM_INFO* item, ANIM_FRAME* framePtr[], int* rate);
ANIM_FRAME* GetBestFrame(ITEM_INFO* item);
BOUNDING_BOX* GetBoundsAccurate(ITEM_INFO* item);
void GetLaraJointPosition(PHD_VECTOR* pos, int LM_enum);
void GetJointAbsPosition(ITEM_INFO* item, PHD_VECTOR* vec, int joint);
void ClampRotation(PHD_3DPOS* pos, short angle, short rot); 
bool TestLastFrame(ITEM_INFO* item, int animNumber = -1);

void DrawAnimatingItem(ITEM_INFO* item);