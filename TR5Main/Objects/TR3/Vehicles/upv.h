#pragma once
#include "items.h"
#include "collide.h"

struct SUB_INFO
{
	int Vel;
	int Rot;
	int RotX;
	short FanRot;
	char Flags;
	char WeaponTimer;
};

void SubInitialise(short itemNum);
void SubCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
int SubControl(void);