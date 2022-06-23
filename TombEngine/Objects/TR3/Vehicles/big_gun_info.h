#pragma once

namespace TEN::Entities::Vehicles
{
	struct BigGunInfo
	{
		EulerAngles Rotation;
		short BarrelZRotation;
		short StartYRot;
		long GunRotYAdd;

		unsigned int FireCount;
		bool BarrelRotating;

		char Flags;
	};
}
