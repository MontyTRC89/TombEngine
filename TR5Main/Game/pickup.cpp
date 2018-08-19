#include "pickup.h"
#include "..\Global\global.h"
#include "draw.h"

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

void Inject_Pickup()
{
	INJECT(0x0043A130, DrawAllPickups);
}