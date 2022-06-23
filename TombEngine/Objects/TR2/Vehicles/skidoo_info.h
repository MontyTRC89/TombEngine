#pragma once

namespace TEN::Entities::Vehicles
{
	struct SkidooInfo
	{
		short TurnRate;
		short MomentumAngle;
		short ExtraRotation;

		int LeftVerticalVelocity;
		int RightVerticalVelocity;

		int Pitch;
		int FlashTimer;
		short TrackMesh;
		bool Armed;
	};
}
