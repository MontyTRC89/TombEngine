#pragma once
#include "items.h"


void InitialiseWraith(short itemNumber);
void WraithControl(short itemNumber);
void WraithEffects(int x, int y, int z, short xVel, short yVel, short zVel, int objNumber);
void DrawWraithEffect(int x, int y, int z, short yrot, short objNumber);
void DrawWraith(ITEM_INFO* item);
void KillWraith(ITEM_INFO* item);
