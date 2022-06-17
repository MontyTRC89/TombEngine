#pragma once

namespace TEN::Entities::Vehicles
{
	struct SpeedBoatInfo
	{
		int TurnRate;
		short LeanAngle;
		short ExtraRotation;

		int LeftVerticalVelocity;
		int RightVerticalVelocity;

		int Water;
		int Pitch;
	};
}
