#pragma once

namespace TEN::Entities::Vehicles
{
	struct SkidooInfo
	{
		float TurnRate;
		float MomentumAngle;
		float ExtraRotation;

		int LeftVerticalVelocity;
		int RightVerticalVelocity;

		int Pitch;
		int FlashTimer;
		short TrackMesh;
		bool Armed;
	};
}
