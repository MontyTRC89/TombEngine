#pragma once

namespace TEN::Entities::Vehicles
{
	struct QuadInfo
	{
		float TurnRate;
		float FrontRot;
		float RearRot;
		float MomentumAngle;
		float ExtraRotation;

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
