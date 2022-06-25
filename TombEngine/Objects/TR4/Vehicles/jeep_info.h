#pragma once

namespace TEN::Entities::Vehicles
{
	struct JeepInfo
	{
		int Velocity = 0;
		int VerticalVelocity = 0;

		short TurnRate = 0;
		short MomentumAngle = 0;
		short ExtraRotation = 0;
		short FrontRightWheelRotation = 0;
		short FrontLeftWheelRotation = 0;
		short BackRightWheelRotation = 0;
		short BackLeftWheelRotation = 0;

		int Revs = 0;
		short EngineRevs = 0;
		int Pitch = 0;
		short TrackMesh = 0;

		short Flags = NULL;

		short unknown0 = 0;
		short unknown1 = 0;
		short unknown2 = 0;
	};
}
