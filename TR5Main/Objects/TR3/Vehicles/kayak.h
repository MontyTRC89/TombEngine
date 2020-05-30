#pragma once
#include "items.h"
#include "collide.h"

struct KAYAK_INFO
{
	int Vel;
	int Rot;
	int FallSpeedF;
	int FallSpeedL;
	int FallSpeedR;
	int Water;
	PHD_3DPOS OldPos;
	char Turn;
	char Forward;
	char TrueWater;
	char Flags;
};

void InitialiseKayak(short itemNumber);
void KayakCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
int KayakControl(void);
void DrawKayak(ITEM_INFO* kayak);