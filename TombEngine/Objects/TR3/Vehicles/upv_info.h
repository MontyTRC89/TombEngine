#pragma once

namespace TEN::Entities::Vehicles
{
	struct UPVInfo
	{
		int Velocity;
		int Rot;
		int XRot;
		short FanRot;
		unsigned int HarpoonTimer;
		bool HarpoonLeft;

		char Flags;
	};
}
