#include "framework.h"
#include "health.h"
#include "animation.h"
#include "pickup.h"
#include "lara.h"
#include "camera.h"
#include "level.h"
#include "control/control.h"
#include "Renderer11.h"
using namespace TEN::Renderer;
short PickupX;
short PickupY;
short CurrentPickup;
DISPLAY_PICKUP Pickups[MAX_COLLECTED_PICKUPS];
short PickupVel;
int OldHitPoints = 1000;
int HealthBarTimer = 40;
float HealthBar = OldHitPoints;
float MutateAmount = 0;
int FlashState = 0;
int FlashCount = 0;
int PoisonFlag = 0;
int DashTimer = 0;
extern RendererHUDBar* g_HealthBar;
extern RendererHUDBar* g_DashBar;
extern RendererHUDBar* g_AirBar;

bool EnableSmoothHealthBar = true;

void DrawHealthBarOverlay(int value)
{
	if (CurrentLevel)
	{
		int color2 = 0;
		if (Lara.poisoned)
			color2 = 0xA0A000;
		else
			color2 = 0xA00000;
		g_Renderer.drawBar(value, ::g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, Lara.poisoned);
	}
}

void DrawHealthBar(float value)
{
	if (CurrentLevel)
	{
		g_Renderer.drawBar(value, ::g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, Lara.poisoned);
	}
}

void UpdateHealthBar(int flash)
{
	int hitPoints = LaraItem->hitPoints;

	if (hitPoints < 0)
		hitPoints = 0;
	else if (hitPoints > 1000)
		hitPoints = 1000;

	// OPT: smoothly transition health bar display.
	if (EnableSmoothHealthBar)
	{
		if (OldHitPoints != hitPoints)
		{
			MutateAmount += OldHitPoints - hitPoints;
			OldHitPoints = hitPoints;
			HealthBarTimer = 40;
		}

		if (HealthBar - MutateAmount < 0)
			MutateAmount = HealthBar;
		else if (HealthBar - MutateAmount > 1000)
			MutateAmount = HealthBar - 1000;

		HealthBar -= MutateAmount / 3;
		MutateAmount -= MutateAmount / 3;

		if (MutateAmount > -0.5f && MutateAmount < 0.5f)
		{
			MutateAmount = 0;
			HealthBar = hitPoints;
		}
	}

	// OG: discretely transition health bar display.
	else
	{
		if (OldHitPoints != hitPoints)
		{
			OldHitPoints = hitPoints;
			HealthBar = hitPoints;
			HealthBarTimer = 40;
		}
	}

	if (HealthBarTimer < 0)
		HealthBarTimer = 0;

	// Flash when at 1/4 capacity AND HP bar is not transitioning.
	if (HealthBar <= 1000 / 4)
	{
		if (!BinocularRange)
		{
			if (flash)
				DrawHealthBar(HealthBar / 1000.0f);
			else
				DrawHealthBar(0);
		}
		else
		{
			if (flash)
				DrawHealthBarOverlay(HealthBar / 1000.0f);
			else
				DrawHealthBarOverlay(0);
		}
	}
	else if ((HealthBarTimer > 0)
		|| (HealthBar <= 0)
		|| (Lara.gunStatus == LG_READY && Lara.gunType != WEAPON_TORCH)
		|| (Lara.poisoned >= 256))
	{
		if (!BinocularRange && !SniperOverlay)
		{
			DrawHealthBar(HealthBar / 1000.0f);
		}
		else
		{
			DrawHealthBarOverlay(HealthBar / 1000.0f);
		}
	}

	if (PoisonFlag)
		PoisonFlag--;
}

void DrawAirBar(float value)
{
	if (CurrentLevel)
	{
		g_Renderer.drawBar(value, ::g_AirBar,ID_AIR_BAR_TEXTURE,0,0);
	}
}

void UpdateAirBar(int flash)
{
	if (Lara.air == 1800 || LaraItem->hitPoints <= 0)
		return;

	if ((Lara.Vehicle == NO_ITEM)
		|| (g_Level.Items[Lara.Vehicle].objectNumber != ID_UPV))
	{
		if ((Lara.waterStatus != LW_UNDERWATER)
			&& (Lara.waterStatus != LW_SURFACE)
			&& (!((g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_SWAMP)
				&& (Lara.waterSurfaceDist < -775))))
			return;
	}

	int air = Lara.air;
	if (air < 0)
		air = 0;
	else if (air > 1800)
		air = 1800;
	if (air <= 450)
	{
		if (flash)
			DrawAirBar(air / 1800.0f);
		else
			DrawAirBar(0);
	}
	else
		DrawAirBar(air / 1800.0f);
}

void DrawDashBar(int value)
{
	if (CurrentLevel)
	{
		g_Renderer.drawBar(value, ::g_DashBar,ID_DASH_BAR_TEXTURE,0,0);
	}
}

void DrawAllPickups()
{
	DISPLAY_PICKUP* pickup = &Pickups[CurrentPickup];

	if (pickup->life > 0)
	{
		if (PickupX > 0)
			PickupX += -PickupX >> 3;
		else
			pickup->life--;
	}
	else if (pickup->life == 0)
	{
		if (PickupX < 128)
		{
			if (PickupVel < 16)
				PickupVel++;

			PickupX += PickupVel;
		}
		else
		{
			pickup->life = -1;
			PickupVel = 0;
		}
	}
	else
	{
		int i;
		for (i = 0; i < MAX_COLLECTED_PICKUPS; i++)
		{
			if (Pickups[CurrentPickup].life > 0)
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

	g_Renderer.drawPickup(Pickups[CurrentPickup].objectNumber);
}


void AddDisplayPickup(GAME_OBJECT_ID objectNumber)
{
	DISPLAY_PICKUP* pickup = Pickups;

	for (int i = 0; i < MAX_COLLECTED_PICKUPS; i++)
	{
		
		if (pickup->life < 0)
		{
			pickup->life = 45;
			pickup->objectNumber = objectNumber;
			break;
		}

		pickup++;
	}

	// No free slot found, so just pickup the object ithout displaying it
	PickedUpObject(objectNumber, 0);
}

void InitialisePickupDisplay()
{
	for (int i = 0; i < MAX_COLLECTED_PICKUPS; i++)
		Pickups[i].life = -1;

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
