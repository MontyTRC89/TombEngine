#pragma once

namespace TEN::Entities::Vehicles
{
	struct UPVInfo
	{
		int Velocity;
		float Rot;
		float XRot;
		float FanRot;
		unsigned int HarpoonTimer;
		bool HarpoonLeft;

		char Flags;
	};
}
