#pragma once

namespace TEN::Entities::Vehicles
{
	struct SpeedBoatInfo
	{
		float TurnRate;
		float LeanAngle;
		float ExtraRotation;

		int LeftVerticalVelocity;
		int RightVerticalVelocity;

		int Water;
		int Pitch;
	};
}
