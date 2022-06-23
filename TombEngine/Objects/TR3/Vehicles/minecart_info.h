#pragma once

namespace TEN::Entities::Vehicles
{
	struct MinecartInfo
	{
		float TurnRot;
		float TurnX;
		float TurnZ;
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
