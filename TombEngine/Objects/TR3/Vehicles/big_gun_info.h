#pragma once

namespace TEN::Entities::Vehicles
{
	struct BigGunInfo
	{
		EulerAngles BaseOrientation = EulerAngles::Zero;
		EulerAngles TurnRate = EulerAngles::Zero;
		EulerAngles Rotation = EulerAngles::Zero;
		short BarrelRotation = 0;
		int XOrientFrame = 0;

		unsigned int FireCount = 0;
		bool IsBarrelRotating = false;

		char Flags = NULL;
	};
}
