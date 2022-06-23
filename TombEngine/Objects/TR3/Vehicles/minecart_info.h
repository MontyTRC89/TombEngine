#pragma once

namespace TEN::Entities::Vehicles
{
	struct MinecartInfo
	{
		short TurnRot;
		int TurnX;
		int TurnZ;
		short TurnLen;

		int Velocity;
		int VerticalVelocity;
		int Gradient;
		unsigned int StopDelay;

		int FloorHeightMiddle;
		int FloorHeightFront;

		char Flags;
	};
}
