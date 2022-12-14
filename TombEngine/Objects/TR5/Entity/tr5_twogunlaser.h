#pragma once
#include "Game/collision/collide_room.h"

class Pose;

namespace TEN::Entities::Creatures::TR5
{

	struct TWOGUNINFO
	{
		Pose pos;
		short life;
		short coil;
		short spin, spinadd;
		short length, dlength;
		short size;
		int r, g, b;
		char fadein;
	};

	extern TWOGUNINFO twogun[2];
	

	void InitialiseTwogun(short itemNumber);
	void TwogunControl(short itemNumber);
	void UpdateTwogunLasers();
	//void DrawTwogunLasers();
	void TriggerTwogunPlasma(const Vector3i& posr, const Pose& pos, float life);

	void FireTwogunWeapon(short itemNumber, short LeftRight, short plasma);

}
//#define ControlGunTestStation	( (void(__cdecl*)(ITEM_INFO*)) 0x0048D940 )
//#define UpdateTwogunLasers	( (void(__cdecl*)()) 0x0048D7D0 )
//#define FireTwogunWeapon	( (void(__cdecl*)(ITEM_INFO*, long, long)) 0x0048DF60 )
//#define DrawTwogunLasers	( (void(__cdecl*)()) 0x0048D900 )
