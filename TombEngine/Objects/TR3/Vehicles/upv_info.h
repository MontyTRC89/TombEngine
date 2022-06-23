#pragma once

namespace TEN::Entities::Vehicles
{
	struct UPVInfo
	{
		Vector3Shrt TurnRate;
		short FanRotation = 0;

		int Velocity = 0;

		unsigned int HarpoonTimer = 0;
		bool HarpoonLeft = false;

		char Flags = NULL;
	};
}
