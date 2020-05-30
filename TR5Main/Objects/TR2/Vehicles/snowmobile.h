#pragma once
#include "items.h"
#include "collide.h"

typedef struct SKIDOO_INFO
{
	short track_mesh;
	int skidoo_turn;
	int left_fallspeed, right_fallspeed;
	short momentum_angle, extra_rotation;
	int pitch;
	bool already_cd_played;
	bool armed;
	int flash_timer;
};

void InitialiseSkidoo(short itemNum);
void SkidooCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll);
int SkidooControl(void);
void DrawSkidoo(ITEM_INFO* item);
void DoSnowEffect(ITEM_INFO* skidoo);