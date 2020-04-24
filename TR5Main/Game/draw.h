#pragma once

#include "..\Global\global.h"

extern Renderer11* g_Renderer;
extern BITE_INFO EnemyBites[9];
extern int LightningCount;
extern int LightningRand;
extern int StormTimer;
extern int dLightningRand;
extern byte SkyStormColor[3];
extern byte SkyStormColor2[3];
extern int GnFrameCounter;

int DrawPhaseGame();
int GetFrame_D2(ITEM_INFO* item, short* framePtr[], int* rate);
void UpdateStorm();
short* GetBoundsAccurate(ITEM_INFO* item);
short* GetBestFrame(ITEM_INFO* item);
int Sync();
bool TIME_Init();
bool TIME_Reset();
void DrawAnimatingItem(ITEM_INFO* item);
void phd_RotBoundingBoxNoPersp(PHD_3DPOS* pos, short* bounds, short* tbounds);
void GetLaraJointPosition(PHD_VECTOR* pos, int joint);