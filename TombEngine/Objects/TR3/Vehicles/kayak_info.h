#pragma once
#include "Specific/phd_global.h"

namespace TEN::Entities::Vehicles
{
	struct KayakInfo
	{
		int TurnRate = 0;

		int Velocity = 0;
		int FrontVerticalVelocity = 0;
		int LeftVerticalVelocity = 0;
		int RightVerticalVelocity = 0;

		PHD_3DPOS OldPose = PHD_3DPOS();
		unsigned int LeftRightPaddleCount = 0;
		int WaterHeight = 0;
		bool Turn = false;
		bool Forward = false;
		bool TrueWater = false;

		int CurrentStartWake = 0;
		int WakeShade = 0;

		char Flags = NULL;
	};
}
