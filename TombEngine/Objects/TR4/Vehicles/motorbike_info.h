#pragma once

namespace TEN::Entities::Vehicles
{
	struct MotorbikeInfo
	{
		short TurnRate = 0;
		short MomentumAngle = 0;
		short ExtraRotation = 0;
		short WallShiftRotation = 0;

		int Velocity = 0;

		int Revs = 0;
		int EngineRevs = 0;
		int Pitch = 0;

		short LightPower = 0;
		short LeftWheelRotation = 0;
		short RightWheelsRotation = 0;

		char ExhaustStart = 0;
		bool DisableDismount = false;

		short Flags = NULL;
	};
}
