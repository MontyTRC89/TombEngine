#pragma once
#include "Box.h"
struct PHD_3DPOS;
struct PHD_VECTOR;
struct ANIM_FRAME;
struct ITEM_INFO;

extern BITE_INFO EnemyBites[9];
extern int LightningCount;
extern int LightningRand;
extern int StormTimer;
extern int dLightningRand;
extern byte SkyStormColor[3];
extern byte SkyStormColor2[3];

int DrawPhaseGame();
int GetFrame_D2(ITEM_INFO* item, ANIM_FRAME* framePtr[], int* rate);
void UpdateStorm();
BOUNDING_BOX* GetBoundsAccurate(ITEM_INFO* item);
ANIM_FRAME* GetBestFrame(ITEM_INFO* item);
int Sync();
bool TIME_Init();
bool TIME_Reset();
void DrawAnimatingItem(ITEM_INFO* item);
void GetLaraJointPosition(PHD_VECTOR* pos, int LM_enum);
void ClampRotation(PHD_3DPOS* pos, short angle, short rot);