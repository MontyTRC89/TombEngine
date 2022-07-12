#pragma once

namespace TEN::Entities::Vehicles
{
	struct UPVInfo
	{
		Vector3Shrt TurnRate;
		short TurbineRotation = 0;
		short LeftRudderRotation = 0;
		short RightRudderRotation = 0;

		int Velocity = 0;

		unsigned int HarpoonTimer = 0;
		bool HarpoonLeft = false;

		char Flags = NULL;
	};
}
