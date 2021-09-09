#pragma once
#include "box.h"

extern BITE_INFO EnemyBites[9];

int DrawPhaseGame();
int GetFrame_D2(ITEM_INFO* item, ANIM_FRAME* framePtr[], int* rate);
BOUNDING_BOX* GetBoundsAccurate(ITEM_INFO* item);
ANIM_FRAME* GetBestFrame(ITEM_INFO* item);
int Sync();
bool TIME_Init();
bool TIME_Reset();
void DrawAnimatingItem(ITEM_INFO* item);
void GetLaraJointPosition(PHD_VECTOR* pos, int LM_enum);
void ClampRotation(PHD_3DPOS* pos, short angle, short rot);