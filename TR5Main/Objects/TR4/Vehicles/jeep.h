#pragma once
#include "items.h"
#include "collide.h"
struct JEEP_INFO {
	short rot1;
	short rot2;
	short rot3;
	short rot4;
	int velocity;
	int revs;
	short engineRevs;
	short trackMesh;
	int jeepTurn;
	int fallSpeed;
	short momentumAngle;
	short extraRotation;
	short unknown0;
	int pitch;
	short flags;
	short unknown1;
	short unknown2;
};
void InitialiseJeep(short itemNumber);
void JeepCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
int JeepControl(void);