#pragma once

namespace TEN::Entities::Vehicles
{
	struct SkidooInfo
	{
		short TurnRate = 0;
		short MomentumAngle = 0;
		short ExtraRotation = 0;

		int LeftVerticalVelocity = 0;
		int RightVerticalVelocity = 0;

		int Pitch = 0;
		int FlashTimer = 0;
		bool Armed = false;
		short TrackMesh = 0;
	};
}
