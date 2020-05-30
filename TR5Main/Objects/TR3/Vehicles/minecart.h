#pragma once
#include "items.h"
#include "collide.h"

typedef struct CART_INFO
{
	int Speed;
	int MidPos;
	int FrontPos;
	int TurnX;
	int TurnZ;
	short TurnLen;
	short TurnRot;
	short YVel;
	short Gradient;
	char Flags;
	char StopDelay;
};

void InitialiseMineCart(short itemNum);
void MineCartCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
int MineCartControl(void);