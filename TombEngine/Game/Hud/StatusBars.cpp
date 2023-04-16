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
extern TEN::Renderer::RendererHudBar* g_ExposureBar;
extern TEN::Renderer::RendererHudBar* g_HealthBar;
extern TEN::Renderer::RendererHudBar* g_StaminaBar;

namespace TEN::Hud
{
	void StatusBarsController::Initialize(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

		// Initialize bar values.
		InitializeStatusBar(AirBar, player.Status.Air, LARA_AIR_MAX);
		InitializeStatusBar(ExposureBar, player.Status.Exposure, LARA_EXPOSURE_MAX);
		InitializeStatusBar(HealthBar, item.HitPoints, LARA_HEALTH_MAX);
		InitializeStatusBar(StaminaBar, player.Status.Stamina, LARA_STAMINA_MAX);
	}

	void StatusBarsController::Update(const ItemInfo& item)
	{
		constexpr auto FLASH_INTERVAL = 0.2f;

		// Update flash.
		if ((GameTimer % (int)round(FLASH_INTERVAL * FPS)) == 0)
			DoFlash = !DoFlash;

		// Update bars.
		UpdateAirBar(item);
		UpdateExposureBar(item);
		UpdateHealthBar(item);
		UpdateStaminaBar(item);
	}

	void StatusBarsController::Draw(const ItemInfo& item) const
	{
		// Avoid drawing in title level and during cutscenes.
		if (CurrentLevel == 0 || CinematicBarsHeight > 0)
			return;

		const auto& player = GetLaraInfo(item);
		bool isPoisoned = (player.Status.Poison != 0);

		// Draw bars.
		DrawAirBar();
		DrawExposureBar();
		DrawHealthBar(isPoisoned);
		DrawStaminaBar();
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
		UpdateStatusBar(AirBar, player.Status.Air, LARA_AIR_MAX);
		
		// Update life.
		if (AirBar.Value != AirBar.TargetValue ||
			player.Control.WaterStatus == WaterStatus::Underwater)
		{
			AirBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}

		// HACK: Special case for UPV as it sets player.Control.WaterStatus to WaterStatus::Dry.
		if (player.Context.Vehicle != NO_ITEM)
		{
			const auto& vehicleItem = g_Level.Items[player.Context.Vehicle];
			if (vehicleItem.ObjectNumber == ID_UPV)
				AirBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}
	}

	void StatusBarsController::UpdateExposureBar(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

		// Update generic data.
		UpdateStatusBar(ExposureBar, player.Status.Exposure, LARA_EXPOSURE_MAX);

		// Update life.
		if (ExposureBar.Value != ExposureBar.TargetValue ||
			(TestEnvironment(ENV_FLAG_WATER, item.RoomNumber) && TestEnvironment(ENV_FLAG_COLD, item.RoomNumber)))
		{
			ExposureBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}
	}

	void StatusBarsController::UpdateHealthBar(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

		// Update generic data.
		UpdateStatusBar(HealthBar, item.HitPoints, LARA_HEALTH_MAX);

		// Update life.
		if (HealthBar.Value != HealthBar.TargetValue ||
			item.HitPoints <= LARA_HEALTH_CRITICAL || player.Status.Poison != 0 ||
			(player.Control.HandStatus == HandStatus::WeaponDraw &&
				player.Control.Weapon.GunType != LaraWeaponType::Flare) || // HACK: Exclude flare.
			(player.Control.HandStatus == HandStatus::WeaponReady &&
				player.Control.Weapon.GunType != LaraWeaponType::Torch))   // HACK: Exclude torch.
		{
			HealthBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}

		// Special case for weapon undraw.
		if (HealthBar.Value == HealthBar.TargetValue &&
			item.HitPoints > LARA_HEALTH_CRITICAL && player.Status.Poison == 0 &&
			player.Control.HandStatus == HandStatus::WeaponUndraw)
		{
			HealthBar.Life = 0;// round(STATUS_BAR_LIFE_START_FADING * FPS);
		}
	}

	void StatusBarsController::UpdateStaminaBar(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

		// Update generic data.
		UpdateStatusBar(StaminaBar, player.Status.Stamina, LARA_STAMINA_MAX);

		// Update life.
		if (StaminaBar.Value != StaminaBar.TargetValue ||
			StaminaBar.Value != 1.0f)
		{
			StaminaBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
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

		if (AirBar.Life <= 0.0f)
			return;

		DrawStatusBar(AirBar.Value, CRITICAL_VALUE, *g_AirBar, TEXTURE_ID, 0, false);
	}
	
	void StatusBarsController::DrawExposureBar() const
	{
		constexpr auto TEXTURE_ID	  = ID_SFX_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = LARA_EXPOSURE_CRITICAL / LARA_EXPOSURE_MAX;

		if (ExposureBar.Life <= 0.0f)
			return;

		DrawStatusBar(ExposureBar.Value, CRITICAL_VALUE, *g_ExposureBar, TEXTURE_ID, 0, false);
	}

	void StatusBarsController::DrawHealthBar(bool isPoisoned) const
	{
		constexpr auto TEXTURE_ID	  = ID_HEALTH_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = LARA_HEALTH_CRITICAL / LARA_HEALTH_MAX;

		if (HealthBar.Life <= 0.0f)
			return;

		DrawStatusBar(HealthBar.Value, CRITICAL_VALUE, *g_HealthBar, TEXTURE_ID, GlobalCounter, isPoisoned);
	}

	void StatusBarsController::DrawStaminaBar() const
	{
		constexpr auto TEXTURE_ID	  = ID_DASH_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = 0;

		if (StaminaBar.Life <= 0.0f)
			return;

		DrawStatusBar(StaminaBar.Value, CRITICAL_VALUE, *g_StaminaBar, TEXTURE_ID, 0, false);
	}
}
