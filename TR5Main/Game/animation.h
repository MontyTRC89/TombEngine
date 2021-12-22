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

struct ANIM_STRUCT
{
	int framePtr;
	short interpolation;
	short currentAnimState;
	int velocity;
	int acceleration;
	int Xvelocity;
	int Xacceleration;
	short frameBase;
	short frameEnd;
	short jumpAnimNum;
	short jumpFrameNum;
	short numberChanges;
	short changeIndex;
	short numberCommands;
	short commandIndex;
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
void SetAnimation(ITEM_INFO* item, short animIndex, short frameToStart = 0);
short GetFrameNumber(ITEM_INFO* item, short frameToStart);
short GetFrameNumber(short objectID, short animNumber, short frameToStart);
int GetFrameCount(short animNumber);
int GetNextAnimState(ITEM_INFO* item);
int GetNextAnimState(short objectID, short animNumber);
int GetChange(ITEM_INFO* item, ANIM_STRUCT* anim);
int GetFrame(ITEM_INFO* item, ANIM_FRAME* framePtr[], int* rate);
ANIM_FRAME* GetBestFrame(ITEM_INFO* item);
BOUNDING_BOX* GetBoundsAccurate(ITEM_INFO* item);
void GetLaraJointPosition(PHD_VECTOR* pos, int LM_enum);
void GetJointAbsPosition(ITEM_INFO* item, PHD_VECTOR* vec, int joint);
void ClampRotation(PHD_3DPOS* pos, short angle, short rot); 
bool TestLastFrame(ITEM_INFO* item, short animNumber = -1);

void DrawAnimatingItem(ITEM_INFO* item);