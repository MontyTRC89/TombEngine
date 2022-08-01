#pragma once

namespace TEN::Entities::Vehicles
{
	struct SkidooInfo
	{
		int LeftVerticalVelocity = 0;
		int RightVerticalVelocity = 0;

		float TurnRate = 0;
		float MomentumAngle = 0;
		float ExtraRotation = 0;

		int Pitch = 0;
		int FlashTimer = 0;
		short TrackMesh = 0;
		bool Armed = false;
	};
}
