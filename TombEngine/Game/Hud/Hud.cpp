#include "framework.h"
#include "Game/Hud/Hud.h"

#include "Game/Hud/PickupSummary.h"
#include "Game/Hud/StatusBars.h"
#include "Game/items.h"

using namespace TEN::Math;

namespace TEN::Hud
{
	HudController g_Hud = {};

	void HudController::Update(const ItemInfo& item)
	{
		TargetHighlighter.Update(item);
		PickupSummary.Update();
		StatusBars.Update(item);
	}

	void HudController::Draw(const ItemInfo& item) const
	{
		TargetHighlighter.Draw();
		PickupSummary.Draw();
		StatusBars.Draw(item);
	}

	void HudController::Clear()
	{
		TargetHighlighter.Clear();
		PickupSummary.Clear();
		StatusBars.Clear();
	}
}
