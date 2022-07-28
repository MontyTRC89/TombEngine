#pragma once
#include "Game/items.h"

namespace TEN::Entities::TR3
{
	void ControlLaserBolts(short itemNumber);
	void ControlLondBossPlasmaBall(short fxNumber);
	void InitialiseLondonBoss(short itemNumber);
	void LondonBossControl(short itemNumber);
	void S_DrawLondonBoss(ItemInfo* item);
}
