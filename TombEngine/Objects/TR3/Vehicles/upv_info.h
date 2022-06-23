#pragma once

namespace TEN::Entities::Vehicles
{
	struct UPVInfo
	{
		int Velocity = 0;

		int Rot = 0;
		int XRot = 0;
		short FanRot = 0;

		unsigned int HarpoonTimer = 0;
		bool HarpoonLeft = false;

		char Flags = NULL;
	};
}
