#pragma once
#include "items.h"

struct WRAITH_INFO
{
	int xPos;
	int yPos;
	int zPos;
	short xRot;
	short yRot;
	short zRot;
	byte r;
	byte g;
	byte b;
};

void InitialiseWraith(short itemNumber);
void WraithControl(short itemNumber);
void WraithWallsEffect(int x, int y, int z, short yrot, short objNumber);
void DrawWraith(int x, int y, int z, short xVel, short yVel, short zVel, int objNumber);
void DrawWraith(ITEM_INFO* item);
void KillWraith(ITEM_INFO* item);
void WraithExplosionEffect(ITEM_INFO* item, byte r, byte g, byte b, int speed);