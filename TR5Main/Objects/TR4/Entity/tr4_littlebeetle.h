#pragma once
#include <items.h>

struct SCARAB_INFO
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
	constexpr auto NUM_LITTLE_BETTLES = 256;

	extern SCARAB_INFO* Scarabs;
	extern int NextScarab;

	void InitialiseScarabs(short itemNumber);
	void ScarabsControl(short itemNumber);
	short GetFreeScarab();
	void ClearScarabs();
	void UpdateScarabs();
}