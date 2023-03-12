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
		this->PickupSummary.Update();
		this->StatusBars.Update(item);
	}

	void HudController::Draw(ItemInfo& item) const
	{
		// ----------------Debug
		static short orient2D = 0;
		orient2D += ANGLE(1.0f);

		auto pos = SCREEN_SPACE_RES / 2;
		g_Renderer.DrawSpriteIn2DSpace(
			ID_BINOCULAR_GRAPHIC,
			pos, orient2D, Vector4(1, 1, 1, 0.5f), Vector2(300.0f));

		//------------------

		this->PickupSummary.Draw();
		this->StatusBars.Draw(item);
	}

	void HudController::Clear()
	{
		this->PickupSummary.Clear();
		this->StatusBars.Clear();
	}
}
