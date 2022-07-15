#pragma once

namespace TEN::Entities::Vehicles
{
	struct BigGunInfo
	{
		EulerAngles BaseOrientation = EulerAngles();
		EulerAngles TurnRate = EulerAngles();
		EulerAngles Rotation = EulerAngles();
		short BarrelRotation = 0;
		int XOrientFrame = 0;

		unsigned int FireCount = 0;
		bool IsBarrelRotating = false;

		char Flags = NULL;
	};
}
