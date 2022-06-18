#pragma once

namespace TEN::Entities::Vehicles
{
	struct QuadBikeInfo
	{
		short TurnRate;
		short FrontRot;
		short RearRot;
		short MomentumAngle;
		short ExtraRotation;

		int Velocity;
		int LeftVerticalVelocity;
		int RightVerticalVelocity;

		int Revs;
		int EngineRevs;
		int Pitch;

		int SmokeStart;
		bool CanStartDrift;
		bool DriftStarting;
		bool NoDismount;

		char Flags;
	};
}
