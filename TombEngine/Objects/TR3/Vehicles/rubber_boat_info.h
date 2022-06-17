#pragma once

namespace TEN::Entities::Vehicles
{
	struct RubberBoatInfo
	{
		int TurnRate;
		short LeanAngle;
		short PropellerRotation;
		short ExtraRotation;

		int LeftVerticalVelocity;
		int RightVerticalVelocity;

		int Water;
		int Pitch;
	};
}
