#pragma once

namespace TEN::Entities::Vehicles
{
	struct SpeedboatInfo
	{
		short TurnRate = 0;
		short LeanAngle = 0;
		short ExtraRotation = 0;

		int LeftVerticalVelocity = 0;
		int RightVerticalVelocity = 0;

		int Pitch = 0;
		int Water = 0;
	};
}
