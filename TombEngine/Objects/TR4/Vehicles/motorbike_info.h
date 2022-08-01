#pragma once

namespace TEN::Entities::Vehicles
{
	struct MotorbikeInfo
	{
		int Velocity = 0;

		float TurnRate = 0;
		float MomentumAngle = 0;
		float ExtraRotation = 0;
		float WallShiftRotation = 0;

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
