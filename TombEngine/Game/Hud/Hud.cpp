#include "framework.h"
#include "Game/Hud/Hud.h"

#include "Game/Hud/PickupSummary.h"
#include "Game/Hud/StatusBars.h"
#include "Game/items.h"

using namespace TEN::Math;

namespace TEN::Hud
{
	HudController g_Hud = {};

	void HudController::Update(ItemInfo& item)
	{
		PickupSummary.Update();
		StatusBars.Update(item);
	}

	void HudController::Draw(ItemInfo& item) const
	{
		PickupSummary.Draw();
		StatusBars.Draw(item);
	}

	void HudController::Clear()
	{
		PickupSummary.Clear();
		StatusBars.Clear();
	}
}
