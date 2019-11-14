#include "healt.h"
#include "draw.h"
#include "pickup.h"

__int16 PickupX;
__int16 PickupY;
__int16 CurrentPickup;
DISPLAY_PICKUP Pickups[NUM_PICKUPS];
__int16 PickupVel;

void __cdecl DrawHealtBar(__int32 percentual)
{
	g_Renderer->DrawHealthBar(percentual);
}

void __cdecl DrawAirBar(__int32 percentual)
{
	g_Renderer->DrawAirBar(percentual);
}

__int32 __cdecl DrawAllPickups()
{
	if (Pickups[CurrentPickup].life > 0)
	{
		if (PickupX > 0)
		{
			PickupX += -PickupX >> 5;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
		else
		{
			Pickups[CurrentPickup].life--;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
	}
	else if (Pickups[CurrentPickup].life == 0)
	{
		if (PickupX < 128)
		{
			if (PickupVel < 16)
				PickupVel++;
			PickupX += PickupVel >> 2;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
		else
		{
			Pickups[CurrentPickup].life = -1;
			PickupVel = 0;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
	}

	__int32 pickupIndex = CurrentPickup;
	__int32 i = 0;
	do
	{
		if (Pickups[pickupIndex].life > 0)
			break;
		pickupIndex = (pickupIndex + 1) & 7;
		i++;
	} while (i < 8);

	CurrentPickup = pickupIndex;
	if (i != 8)
		return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);

	CurrentPickup = 0;

	return 0;
}


void __cdecl AddDisplayPickup(__int16 objectNumber)
{
	for (__int32 i = 0; i < 8; i++)
	{
		DISPLAY_PICKUP* pickup = &Pickups[i];
		if (pickup->life < 0)
		{
			pickup->life = 45;
			pickup->objectNumber = objectNumber;
			PickedUpObject(objectNumber);
			return;
		}
	}

	// No free slot found, so just pickup the object ithout displaying it
	PickedUpObject(objectNumber);
}

void __cdecl InitialisePickUpDisplay()
{
	for (__int32 i = 0; i < 8; i++)
	{
		DISPLAY_PICKUP* pickup = &Pickups[i];
		pickup->life = -1;
	}

	PickupX = 128;
	PickupY = 128;
	PickupVel = 0;
	CurrentPickup = 0;
}

void Inject_Healt()
{
	INJECT(0x004B1950, DrawHealtBar);
	INJECT(0x004B18E0, DrawAirBar);
}
