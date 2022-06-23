#pragma once

namespace TEN::Entities::Vehicles
{
	struct BigGunInfo
	{
		Vector3Shrt Rotation;
		short BarrelZRotation;
		short StartYRot;
		long GunRotYAdd;

		unsigned int FireCount;
		bool BarrelRotating;

		char Flags;
	};
}
