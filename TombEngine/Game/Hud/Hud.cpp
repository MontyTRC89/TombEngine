#include "Game/Hud/Hud.h"

#include "Game/Hud/PickupSummary.h"
#include "Game/Hud/StatusBars.h"
#include "Game/items.h"

namespace TEN::Hud
{
	HudController g_Hud = {};

	void HudController::Update(const ItemInfo& playerItem)
	{
		TargetHighlighter.Update(playerItem);
		Speedometer.Update();
		PickupSummary.Update();
		StatusBars.Update(playerItem);
	}

	void HudController::Draw(const ItemInfo& playerItem) const
	{
		TargetHighlighter.Draw();
		Speedometer.Draw();
		PickupSummary.Draw();
		StatusBars.Draw(playerItem);
	}

	void HudController::Clear()
	{
		TargetHighlighter.Clear();
		Speedometer.Clear();
		PickupSummary.Clear();
		StatusBars.Clear();
	}
}
