#pragma once
#include "box.h"

struct PHD_3DPOS;
struct PHD_VECTOR;
struct ANIM_FRAME;
struct ITEM_INFO;

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

void AnimateItem(ITEM_INFO* item);
void TranslateItem(ITEM_INFO* item, int x, int y, int z);
int GetChange(ITEM_INFO* item, ANIM_STRUCT* anim);
int GetFrame(ITEM_INFO* item, ANIM_FRAME* framePtr[], int* rate);
ANIM_FRAME* GetBestFrame(ITEM_INFO* item);
BOUNDING_BOX* GetBoundsAccurate(ITEM_INFO* item);
void GetLaraJointPosition(PHD_VECTOR* pos, int LM_enum);
void GetJointAbsPosition(ITEM_INFO* item, PHD_VECTOR* vec, int joint);
void ClampRotation(PHD_3DPOS* pos, short angle, short rot);

void DrawAnimatingItem(ITEM_INFO* item);