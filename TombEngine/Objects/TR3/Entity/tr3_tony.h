#pragma once
#include "Game/items.h"

namespace TEN::Entities::TR3
{
	void InitialiseTony(short itemNumber);
	void TonyControl(short itemNumber);
	void ControlTonyFireBall(short fxNumber);
	void S_DrawTonyBoss(ItemInfo* item);
}
