#include "framework.h"
#include "Game/Hud/StatusBars.h"

#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/game_object_ids.h"
#include "Renderer/Renderer11.h"
#include "Specific/clock.h"

using namespace TEN::Renderer;

extern TEN::Renderer::RendererHudBar* g_AirBar;
extern TEN::Renderer::RendererHudBar* g_ColdBar;
extern TEN::Renderer::RendererHudBar* g_HealthBar;
extern TEN::Renderer::RendererHudBar* g_SprintBar;

namespace TEN::Hud
{
	void StatusBarsController::Initialize(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

		// Initialize bar values.
		this->InitializeStatusBar(this->AirBar, player.Status.Air, LARA_AIR_MAX);
		this->InitializeStatusBar(this->ColdBar, player.Status.ColdExposure, LARA_COLD_EXPOSURE_MAX);
		this->InitializeStatusBar(this->HealthBar, item.HitPoints, LARA_HEALTH_MAX);
		this->InitializeStatusBar(this->SprintBar, player.Status.SprintEnergy, LARA_SPRINT_ENERGY_MAX);
	}

	void StatusBarsController::Update(const ItemInfo& item)
	{
		constexpr auto FLASH_INTERVAL = 0.2f;

		// Update flash.
		if ((GameTimer % (int)round(FLASH_INTERVAL * FPS)) == 0)
			this->DoFlash = !DoFlash;

		// Update bars.
		this->UpdateAirBar(item);
		this->UpdateColdBar(item);
		this->UpdateHealthBar(item);
		this->UpdateSprintBar(item);
	}

	void StatusBarsController::Draw(const ItemInfo& item) const
	{
		// Avoid drawing in title level and during cutscenes.
		if (CurrentLevel == 0 || CinematicBarsHeight > 0)
			return;

		const auto& player = GetLaraInfo(item);
		bool isPoisoned = (player.Status.PoisonPotency != 0);

		// Draw bars.
		this->DrawAirBar();
		this->DrawColdBar();
		this->DrawHealthBar(isPoisoned);
		this->DrawSprintBar();
	}

	void StatusBarsController::Clear()
	{
		*this = {};
	}

	void StatusBarsController::InitializeStatusBar(StatusBar& bar, float statusValue, float statusValueMax)
	{
		float statusValueNorm = std::clamp(statusValue, 0.0f, statusValueMax);
		bar.Value = statusValueNorm / statusValueMax;
	}

	void StatusBarsController::UpdateStatusBar(StatusBar& bar, float statusValue, float statusValueMax)
	{
		// Update life.
		if (bar.Life > 0.0f)
			bar.Life -= 1.0f;

		// Update opacity.
		float alpha = std::clamp(bar.Life, 0.0f, STATUS_BAR_LIFE_START_FADING) / STATUS_BAR_LIFE_START_FADING;
		bar.Opacity = Lerp(0.0f, 1.0f, alpha);

		// Update target value.
		float statusValueNorm = std::clamp(statusValue, 0.0f, statusValueMax);
		bar.TargetValue = statusValueNorm / statusValueMax;

		// Update value.
		bar.Value = Lerp(bar.Value, bar.TargetValue, STATUS_BAR_VALUE_LERP_ALPHA);
		if (abs(bar.Value - bar.TargetValue) <= EPSILON)
			bar.Value = bar.TargetValue;
	}

	void StatusBarsController::UpdateAirBar(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

		// Update generic data.
		this->UpdateStatusBar(this->AirBar, player.Status.Air, LARA_AIR_MAX);
		
		// Update life.
		if (AirBar.Value != AirBar.TargetValue ||
			player.Control.WaterStatus == WaterStatus::Underwater)
		{
			this->AirBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}

		// HACK: Special case for UPV as it sets player.Control.WaterStatus to WaterStatus::Dry.
		if (player.Vehicle != NO_ITEM)
		{
			if (g_Level.Items[player.Vehicle].ObjectNumber != ID_UPV)
				this->AirBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}
	}

	void StatusBarsController::UpdateColdBar(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

		// Update generic data.
		this->UpdateStatusBar(this->ColdBar, player.Status.ColdExposure, LARA_COLD_EXPOSURE_MAX);

		// Update life.
		if (ColdBar.Value != ColdBar.TargetValue ||
			TestEnvironment(ENV_FLAG_COLD, &item))
		{
			this->ColdBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}
	}

	void StatusBarsController::UpdateHealthBar(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

		// Update generic data.
		this->UpdateStatusBar(this->HealthBar, item.HitPoints, LARA_HEALTH_MAX);

		// Update life.
		if (HealthBar.Value != HealthBar.TargetValue ||
			item.HitPoints <= LARA_HEALTH_CRITICAL || player.Status.PoisonPotency != 0 ||
			player.Control.HandStatus == HandStatus::WeaponDraw ||
			(player.Control.HandStatus == HandStatus::WeaponReady &&
				player.Control.Weapon.GunType != LaraWeaponType::Torch)) // HACK: Exclude torch.
		{
			this->HealthBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}

		// Special case for weapon undraw.
		if (HealthBar.Value == HealthBar.TargetValue &&
			item.HitPoints > LARA_HEALTH_CRITICAL && player.Status.PoisonPotency == 0 &&
			player.Control.HandStatus == HandStatus::WeaponUndraw)
		{
			this->HealthBar.Life = 0;// round(STATUS_BAR_LIFE_START_FADING * FPS);
		}
	}

	void StatusBarsController::UpdateSprintBar(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

		// Update generic data.
		this->UpdateStatusBar(this->SprintBar, player.Status.SprintEnergy, LARA_SPRINT_ENERGY_MAX);

		// Update life.
		if (SprintBar.Value != SprintBar.TargetValue ||
			SprintBar.Value != 1.0f)
		{
			this->SprintBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}
	}

	void StatusBarsController::DrawStatusBar(float value, float criticalValue, const RendererHudBar& rHudBar, GAME_OBJECT_ID textureID, int frame, bool isPoisoned) const
	{
		if (value <= criticalValue)
			value = DoFlash ? value : 0.0f;

		g_Renderer.DrawBar(value, rHudBar, textureID, frame, isPoisoned);
	}

	void StatusBarsController::DrawAirBar() const
	{
		constexpr auto TEXTURE_ID	  = ID_AIR_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = LARA_AIR_CRITICAL / LARA_AIR_MAX;


		this->DrawStatusBar(AirBar.Value, CRITICAL_VALUE, *g_AirBar, TEXTURE_ID, 0, false);
	}
	
	void StatusBarsController::DrawColdBar() const
	{
		constexpr auto TEXTURE_ID	  = ID_SFX_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = LARA_COLD_EXPOSURE_CRITICAL / LARA_COLD_EXPOSURE_MAX;


		this->DrawStatusBar(ColdBar.Value, CRITICAL_VALUE, *g_ColdBar, TEXTURE_ID, 0, false);
	}

	void StatusBarsController::DrawHealthBar(bool isPoisoned) const
	{
		constexpr auto TEXTURE_ID	  = ID_HEALTH_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = LARA_HEALTH_CRITICAL / LARA_HEALTH_MAX;


		this->DrawStatusBar(HealthBar.Value, CRITICAL_VALUE, *g_HealthBar, TEXTURE_ID, GlobalCounter, isPoisoned);
	}

	void StatusBarsController::DrawSprintBar() const
	{
		constexpr auto TEXTURE_ID	  = ID_DASH_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = 0;


		this->DrawStatusBar(SprintBar.Value, CRITICAL_VALUE, *g_SprintBar, TEXTURE_ID, 0, false);
	}
}
