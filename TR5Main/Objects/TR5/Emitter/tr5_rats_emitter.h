#pragma once
#include "Game/items.h"

constexpr auto NUM_RATS = 32;

struct RAT_STRUCT
{
	PHD_3DPOS pos; // size=20, offset=0
	short roomNumber; // size=0, offset=20
	short speed; // size=0, offset=22
	short fallspeed; // size=0, offset=24
	byte on; // size=0, offset=26
	byte flags; // size=0, offset=27
};

extern int NextRat;
extern RAT_STRUCT Rats[NUM_RATS];

void ClearRats();
short GetNextRat();
void InitialiseLittleRats(short itemNumber);
void LittleRatsControl(short itemNumber);
void UpdateRats();