#pragma once
#include <items.h>

struct SCARAB_STRUCT
{
	PHD_3DPOS pos;
	short roomNumber;
	short speed;
	short fallspeed;
	byte on;
	byte flags;
};

namespace TEN::Entities::TR4
{
	constexpr auto NUM_SCARABS = 256;

	extern SCARAB_STRUCT Scarabs[NUM_SCARABS];
	extern int NextScarab;

	void InitialiseScarabs(short itemNumber);
	void ScarabsControl(short itemNumber);
	short GetFreeScarab();
	void ClearScarabs();
	void UpdateScarabs();
}