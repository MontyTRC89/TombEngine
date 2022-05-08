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

void DrawHealthBarOverlay(ItemInfo* item, int value)
{
	auto* lara = GetLaraInfo(item);

	if (CurrentLevel)
	{
		int color2 = 0;
		if (lara->PoisonPotency)
			color2 = 0xA0A000;
		else
			color2 = 0xA00000;

		g_Renderer.DrawBar(value, ::g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, Lara.PoisonPotency);
	}
}

void DrawHealthBar(ItemInfo* item, float value)
{
	auto* lara = GetLaraInfo(item);

	if (CurrentLevel)
		g_Renderer.DrawBar(value, ::g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, Lara.PoisonPotency);
}

void UpdateHealthBar(ItemInfo* item, int flash)
{
	auto* lara = GetLaraInfo(item);

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
	else if (HealthBarTimer > 0 || HealthBar <= 0 ||
		lara->Control.HandStatus == HandStatus::WeaponReady &&
		lara->Control.Weapon.GunType != LaraWeaponType::Torch ||
		lara->PoisonPotency)
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
		g_Renderer.DrawBar(value, ::g_AirBar,ID_AIR_BAR_TEXTURE,0,0);
}

void UpdateAirBar(ItemInfo* item, int flash)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Air == LARA_AIR_MAX || item->HitPoints <= 0)
		return;

	if (lara->Vehicle == NO_ITEM ||
		g_Level.Items[lara->Vehicle].ObjectNumber != ID_UPV)
	{
		if (lara->Control.WaterStatus != WaterStatus::Underwater &&
			lara->Control.WaterStatus != WaterStatus::TreadWater &&
			!(TestEnvironment(ENV_FLAG_SWAMP, item) && lara->WaterSurfaceDist < -(CLICK(3) - 1)))
			return;
	}

	int air = lara->Air;
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
		g_Renderer.DrawBar(value, ::g_DashBar, ID_DASH_BAR_TEXTURE, 0, 0);
}

void UpdateSprintBar(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->SprintEnergy < LARA_SPRINT_ENERGY_MAX)
		DrawSprintBar(lara->SprintEnergy / LARA_SPRINT_ENERGY_MAX);
}

void DrawAllPickups()
{
	auto* pickup = &Pickups[CurrentPickup];

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
	auto* pickup = Pickups;

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
