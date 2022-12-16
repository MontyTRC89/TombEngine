#include "framework.h"
#include "Objects/Utils/Vehicle.h"

#include "Game/Lara/lara_helpers.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Math/Math.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Input;
using namespace TEN::Math;

namespace TEN::Entities::Vehicles
{
	void AbstractVehicle::SetPlayerVehicle(ItemInfo& item, const ItemInfo* vehicleItem)
	{
		auto& player = *GetLaraInfo(&item);

		if (vehicleItem == nullptr)
		{
			if (player.Vehicle != NO_ITEM)
				g_Level.Items[player.Vehicle].Active = false;

			player.Vehicle = NO_ITEM;
		}
		else
		{
			g_Level.Items[vehicleItem->Index].Active = true;
			player.Vehicle = vehicleItem->Index;
		}
	}

	void AbstractVehicle::ModulateTurnRateX(short accelRate, short minTurnRate, short maxTurnRate, bool invert)
	{
		this->Control.TurnRate.x = ModulateTurnRate(Control.TurnRate.x, accelRate, minTurnRate, maxTurnRate, AxisMap[InputAxis::MoveVertical], invert);
	}

	void AbstractVehicle::ModulateTurnRateY(short accelRate, short minTurnRate, short maxTurnRate, bool invert)
	{
		this->Control.TurnRate.y = ModulateTurnRate(Control.TurnRate.y, accelRate, minTurnRate, maxTurnRate, AxisMap[InputAxis::MoveHorizontal], invert);
	}

	void AbstractVehicle::ResetTurnRateX(short decelRate)
	{
		this->Control.TurnRate.x = ResetTurnRate(Control.TurnRate.x, decelRate);
	}

	void AbstractVehicle::ResetTurnRateY(short decelRate)
	{
		this->Control.TurnRate.y = ResetTurnRate(Control.TurnRate.y, decelRate);
	}

	// TODO: Vehicle turn rates *MUST* be affected by vehicle speed for more tactile modulation.
	short AbstractVehicle::ModulateTurnRate(short turnRate, short accelRate, short minTurnRate, short maxTurnRate, float axisCoeff, bool invert)
	{
		axisCoeff *= invert ? -1 : 1;
		int sign = std::copysign(1, axisCoeff);

		short minTurnRateNormalized = minTurnRate * abs(axisCoeff);
		short maxTurnRateNormalized = maxTurnRate * abs(axisCoeff);

		short newTurnRate = (turnRate + (accelRate * sign)) * sign;
		newTurnRate = std::clamp(newTurnRate, minTurnRateNormalized, maxTurnRateNormalized);
		return (newTurnRate * sign);
	}

	short AbstractVehicle::ResetTurnRate(short turnRate, short decelRate)
	{
		int sign = std::copysign(1, turnRate);

		if (abs(turnRate) <= decelRate)
			return 0;
		else
			return (turnRate + (decelRate * -sign));
	}
}
