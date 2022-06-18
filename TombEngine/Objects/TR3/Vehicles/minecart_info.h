#pragma once

namespace TEN::Entities::Vehicles
{
	struct MinecartInfo
	{
		short TurnRot = 0;
		int TurnX = 0;
		int TurnZ = 0;
		short TurnLen = 0;

		int Velocity = 0;
		int VerticalVelocity = 0;
		int Gradient = 0;
		unsigned int StopDelay = 0;

		int FloorHeightMiddle = 0;
		int FloorHeightFront = 0;

		char Flags = NULL;
	};
}
