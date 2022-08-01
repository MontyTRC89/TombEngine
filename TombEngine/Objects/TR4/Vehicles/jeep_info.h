#pragma once

namespace TEN::Entities::Vehicles
{
	struct JeepInfo
	{
		int Velocity = 0;
		int CameraElevation = 0;

		float TurnRate = 0;
		float MomentumAngle = 0;
		float ExtraRotation = 0;
		float ExtraRotationDrift = 0;
		float FrontRightWheelRotation = 0;
		float FrontLeftWheelRotation = 0;
		float BackRightWheelRotation = 0;
		float BackLeftWheelRotation = 0;

		int Revs = 0;
		short EngineRevs = 0;
		int Pitch = 0;
		short TrackMesh = 0;

		short Gear = 0;
		short Flags = NULL;
	};
}
