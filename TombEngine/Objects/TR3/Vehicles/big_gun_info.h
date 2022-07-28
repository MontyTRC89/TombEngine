#pragma once

namespace TEN::Entities::Vehicles
{
	struct BigGunInfo
	{
		Vector3Shrt BaseOrientation = Vector3Shrt();
		Vector3Shrt TurnRate = Vector3Shrt();
		Vector3Shrt Rotation = Vector3Shrt();
		short BarrelRotation = 0;
		int XOrientFrame = 0;

		unsigned int FireCount = 0;
		bool IsBarrelRotating = false;

		char Flags = NULL;
	};
}
