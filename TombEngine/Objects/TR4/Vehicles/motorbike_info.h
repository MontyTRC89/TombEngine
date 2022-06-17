#pragma once

namespace TEN::Entities::Vehicles
{
	struct MotorbikeInfo
	{
		short TurnRate;
		short MomentumAngle;
		short ExtraRotation;
		short WallShiftRotation;

		int Velocity;

		int Revs;
		int EngineRevs;
		int Pitch;

		short LightPower;
		short LeftWheelRotation;
		short RightWheelsRotation;

		char ExhaustStart;
		bool DisableDismount;

		short Flags;
	};
}
