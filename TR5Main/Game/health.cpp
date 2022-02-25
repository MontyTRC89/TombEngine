#include "framework.h"
#include "Game/health.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Game/pickup/pickup.h"
#include "Renderer/Renderer11.h"
#include "Specific/level.h"

using namespace TEN::Renderer;
short PickupX;
short PickupY;
short CurrentPickup;
DisplayPickup Pickups[MAX_COLLECTED_PICKUPS];
short PickupVel;
int OldHitPoints = LARA_HEALTH_MAX;
int HealthBarTimer = 40;
float HealthBar = OldHitPoints;
float MutateAmount = 0;
int FlashState = 0;
int FlashCount = 0;
int PoisonFlag = 0;
extern RendererHUDBar* g_HealthBar;
extern RendererHUDBar* g_DashBar;
extern RendererHUDBar* g_AirBar;

bool EnableSmoothHealthBar = true;

void DrawHealthBarOverlay(ITEM_INFO* item, int value)
{
	auto* info = GetLaraInfo(item);

	if (CurrentLevel)
	{
		int color2 = 0;
		if (info->poisoned)
			color2 = 0xA0A000;
		else
			color2 = 0xA00000;

		g_Renderer.drawBar(value, ::g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, info->poisoned);
	}
}

void DrawHealthBar(ITEM_INFO* item, float value)
{
	auto* info = GetLaraInfo(item);

	if (CurrentLevel)
		g_Renderer.drawBar(value, ::g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, info->poisoned);
}

void UpdateHealthBar(ITEM_INFO* item, int flash)
{
	auto* info = GetLaraInfo(item);

	auto HitPoints = item->HitPoints;

	if (HitPoints < 0)
		HitPoints = 0;
	else if (HitPoints > LARA_HEALTH_MAX)
		HitPoints = LARA_HEALTH_MAX;

	// OPT: smoothly transition health bar display.
	if (EnableSmoothHealthBar)
	{
		if (OldHitPoints != HitPoints)
		{
			MutateAmount += OldHitPoints - HitPoints;
			OldHitPoints = HitPoints;
			HealthBarTimer = 40;
		}

		if (HealthBar - MutateAmount < 0)
			MutateAmount = HealthBar;
		else if (HealthBar - MutateAmount > LARA_HEALTH_MAX)
			MutateAmount = HealthBar - LARA_HEALTH_MAX;

		HealthBar -= MutateAmount / 3;
		MutateAmount -= MutateAmount / 3;

		if (MutateAmount > -0.5f && MutateAmount < 0.5f)
		{
			MutateAmount = 0;
			HealthBar = HitPoints;
		}
	}

	// OG: discretely transition health bar display.
	else
	{
		if (OldHitPoints != HitPoints)
		{
			OldHitPoints = HitPoints;
			HealthBar = HitPoints;
			HealthBarTimer = 40;
		}
	}

	if (HealthBarTimer < 0)
		HealthBarTimer = 0;

	// Flash when at 1/4 capacity AND HP bar is not transitioning.
	if (HealthBar <= LARA_HEALTH_MAX / 4)
	{
		if (!BinocularRange)
		{
			if (flash)
				DrawHealthBar(item, HealthBar / LARA_HEALTH_MAX);
			else
				DrawHealthBar(item, 0);
		}
		else
		{
			if (flash)
				DrawHealthBarOverlay(item, HealthBar / LARA_HEALTH_MAX);
			else
				DrawHealthBarOverlay(item, 0);
		}
	}
	else if (HealthBarTimer > 0 ||
		HealthBar <= 0 ||
		info->Control.HandStatus == HandStatus::WeaponReady && info->Control.WeaponControl.GunType != WEAPON_TORCH ||
		info->poisoned >= 256)
	{
		if (!BinocularRange)
			DrawHealthBar(item, HealthBar / LARA_HEALTH_MAX);
		else
			DrawHealthBarOverlay(item, HealthBar / LARA_HEALTH_MAX);
	}

	if (PoisonFlag)
		PoisonFlag--;
}

void DrawAirBar(float value)
{
	if (CurrentLevel)
		g_Renderer.drawBar(value, ::g_AirBar,ID_AIR_BAR_TEXTURE,0,0);
}

void UpdateAirBar(ITEM_INFO* item, int flash)
{
	auto* info = GetLaraInfo(item);

	if (info->Air == LARA_AIR_MAX || item->HitPoints <= 0)
		return;

	if (info->Vehicle == NO_ITEM ||
		g_Level.Items[info->Vehicle].ObjectNumber != ID_UPV)
	{
		if (info->Control.WaterStatus != WaterStatus::Underwater &&
			info->Control.WaterStatus != WaterStatus::TreadWater &&
			!(TestEnvironment(ENV_FLAG_SWAMP, item) && info->WaterSurfaceDist < -(CLICK(3) - 1)))
			return;
	}

	int air = info->Air;
	if (air < 0)
		air = 0;
	else if (air > LARA_AIR_MAX)
		air = LARA_AIR_MAX;
	if (air <= (LARA_AIR_MAX / 4))
	{
		if (flash)
			DrawAirBar(air / LARA_AIR_MAX);
		else
			DrawAirBar(0);
	}
	else
		DrawAirBar(air / LARA_AIR_MAX);
}

void DrawSprintBar(float value)
{
	if (CurrentLevel)
		g_Renderer.drawBar(value, ::g_DashBar, ID_DASH_BAR_TEXTURE, 0, 0);
}

void UpdateSprintBar(ITEM_INFO* item)
{
	auto* info = GetLaraInfo(item);

	if (info->SprintEnergy < LARA_SPRINT_MAX)
		DrawSprintBar(info->SprintEnergy / LARA_SPRINT_MAX);
}

void DrawAllPickups()
{
	DisplayPickup* pickup = &Pickups[CurrentPickup];

	if (pickup->Life > 0)
	{
		if (PickupX > 0)
			PickupX += -PickupX >> 3;
		else
			pickup->Life--;
	}
	else if (pickup->Life == 0)
	{
		if (PickupX < 128)
		{
			if (PickupVel < 16)
				PickupVel++;

			PickupX += PickupVel;
		}
		else
		{
			pickup->Life = -1;
			PickupVel = 0;
		}
	}
	else
	{
		int i;
		for (i = 0; i < MAX_COLLECTED_PICKUPS; i++)
		{
			if (Pickups[CurrentPickup].Life > 0)
				break;

			CurrentPickup++;
			CurrentPickup &= (MAX_COLLECTED_PICKUPS - 1);
		}

		if (i == MAX_COLLECTED_PICKUPS)
		{
			CurrentPickup = 0;

			return;
		}
	}

	g_Renderer.drawPickup(Pickups[CurrentPickup].ObjectNumber);
}


void AddDisplayPickup(GAME_OBJECT_ID objectNumber)
{
	DisplayPickup* pickup = Pickups;

	for (int i = 0; i < MAX_COLLECTED_PICKUPS; i++)
	{
		if (pickup->Life < 0)
		{
			pickup->Life = 45;
			pickup->ObjectNumber = objectNumber;

			break;
		}

		pickup++;
	}

	// No free slot found; pickup the object without displaying it.
	PickedUpObject(objectNumber, 0);
}

void InitialisePickupDisplay()
{
	for (int i = 0; i < MAX_COLLECTED_PICKUPS; i++)
		Pickups[i].Life = -1;

	PickupX = 128;
	PickupY = 128;
	PickupVel = 0;
	CurrentPickup = 0;
}

int FlashIt()
{
	if (FlashCount)
		FlashCount--;
	else
	{
		FlashState ^= 1;
		FlashCount = 5;
	}

	return FlashState;
}
