#include "framework.h"
#include "Game/Hud/Hud.h"

#include "Game/control/control.h"
#include "Game/Hud/PickupSummary.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Math;
using namespace TEN::Renderer;

extern RendererHUDBar* g_HealthBar;
extern RendererHUDBar* g_DashBar;
extern RendererHUDBar* g_AirBar;

namespace TEN::Hud
{
	HudController g_Hud = HudController();

	void HudController::Update()
	{
		this->PickupSummary.Update();
	}

	void HudController::Draw() const
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
	}

	void HudController::Clear()
	{
		this->PickupSummary.Clear();
	}

	constexpr auto HEALTH_BAR_TIMER_MAX = 40.0f;

	struct HealthBarData
	{
		float Value		   = LARA_HEALTH_MAX;
		float PrevValue	   = LARA_HEALTH_MAX;
		float MutateAmount = 0.0f;
		float Timer		   = HEALTH_BAR_TIMER_MAX;
	};

	auto HealthBar = HealthBarData();

	static void DrawHealthBarOverlay(ItemInfo* item, float value)
	{
		const auto& player = *GetLaraInfo(item);

		if (CurrentLevel != 0)
			g_Renderer.DrawBar(value, g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, player.PoisonPotency != 0);
	}

	static void DrawHealthBar(ItemInfo* item, float value)
	{
		const auto& player = *GetLaraInfo(item);

		g_Renderer.DrawBar(value, g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, player.PoisonPotency != 0);
	}

	static void DrawHealthBar(ItemInfo* item, bool doFlash)
	{
		const auto& player = *GetLaraInfo(item);

		// Flash when at critical capacity and bar is not transitioning.
		if (HealthBar.Value <= LARA_HEALTH_CRITICAL)
		{
			if (!BinocularRange)
			{
				if (doFlash || HealthBar.MutateAmount)
					DrawHealthBar(item, HealthBar.Value / LARA_HEALTH_MAX);
				else
					DrawHealthBar(item, 0.0f);
			}
			else
			{
				if (doFlash || HealthBar.MutateAmount)
					DrawHealthBarOverlay(item, HealthBar.Value / LARA_HEALTH_MAX);
				else
					DrawHealthBarOverlay(item, 0.0f);
			}
		}
		else if (HealthBar.Timer > 0.0f || HealthBar.Value <= 0.0f ||
			player.Control.HandStatus == HandStatus::WeaponReady &&
			player.Control.Weapon.GunType != LaraWeaponType::Torch ||
			player.PoisonPotency)
		{
			if (!BinocularRange)
				DrawHealthBar(item, HealthBar.Value / LARA_HEALTH_MAX);
			else
				DrawHealthBarOverlay(item, HealthBar.Value / LARA_HEALTH_MAX);
		}
	}

	static void DrawAirBar(float value)
	{
		g_Renderer.DrawBar(value, g_AirBar, ID_AIR_BAR_TEXTURE, 0, false);
	}

	static void DrawAirBar(ItemInfo* item, bool doFlash)
	{
		const auto& player = *GetLaraInfo(item);

		if (player.Air == LARA_AIR_MAX || item->HitPoints <= 0)
			return;

		if (player.Vehicle == NO_ITEM ||
			g_Level.Items[player.Vehicle].ObjectNumber != ID_UPV)
		{
			if (player.Control.WaterStatus != WaterStatus::Underwater &&
				player.Control.WaterStatus != WaterStatus::TreadWater &&
				!(TestEnvironment(ENV_FLAG_SWAMP, item) && player.WaterSurfaceDist < -(CLICK(3) - 1)))
			{
				return;
			}
		}

		float air = player.Air;
		if (air < 0.0f)
		{
			air = 0.0f;
		}
		else if (air > LARA_AIR_MAX)
		{
			air = LARA_AIR_MAX;
		}

		if (air <= LARA_AIR_CRITICAL)
		{
			if (doFlash)
				DrawAirBar(air / LARA_AIR_MAX);
			else
				DrawAirBar(0.0f);
		}
		else
		{
			DrawAirBar(air / LARA_AIR_MAX);
		}
	}

	static void DrawSprintBar(float value)
	{
		g_Renderer.DrawBar(value, g_DashBar, ID_DASH_BAR_TEXTURE, 0, false);
	}

	static void DrawSprintBar(ItemInfo* item)
	{
		const auto& player = *GetLaraInfo(item);

		if (player.SprintEnergy < LARA_SPRINT_ENERGY_MAX)
			DrawSprintBar(player.SprintEnergy / LARA_SPRINT_ENERGY_MAX);
	}

	void UpdateBars(ItemInfo* item)
	{
		const auto& player = *GetLaraInfo(item);

		if (HealthBar.Timer > 0.0f)
			HealthBar.Timer -= 1.0f;

		float hitPoints = item->HitPoints;
		if (hitPoints < 0.0f)
		{
			hitPoints = 0.0f;
		}
		else if (hitPoints > LARA_HEALTH_MAX)
		{
			hitPoints = LARA_HEALTH_MAX;
		}

		// Smoothly transition health bar display.

		if (HealthBar.PrevValue != hitPoints)
		{
			HealthBar.MutateAmount += HealthBar.PrevValue - hitPoints;
			HealthBar.PrevValue = hitPoints;
			HealthBar.Timer = HEALTH_BAR_TIMER_MAX;
		}

		if ((HealthBar.Value - HealthBar.MutateAmount) < 0.0f)
		{
			HealthBar.MutateAmount = HealthBar.Value;
		}
		else if ((HealthBar.Value - HealthBar.MutateAmount) > LARA_HEALTH_MAX)
		{
			HealthBar.MutateAmount = HealthBar.Value - LARA_HEALTH_MAX;
		}

		HealthBar.Value -= HealthBar.MutateAmount / 3;
		HealthBar.MutateAmount -= HealthBar.MutateAmount / 3;

		if (abs(HealthBar.MutateAmount) < 0.5f)
		{
			HealthBar.MutateAmount = 0.0f;
			HealthBar.Value = hitPoints;
		}
	}

	void DrawHud(ItemInfo* item)
	{
		static bool doFlash = false;
		if ((GameTimer & 0x07) == 0x07)
			doFlash = !doFlash;

		if (CurrentLevel == 0 || CinematicBarsHeight > 0)
			return;

		DrawSprintBar(item);
		DrawHealthBar(item, doFlash);
		DrawAirBar(item, doFlash);
	}
}
