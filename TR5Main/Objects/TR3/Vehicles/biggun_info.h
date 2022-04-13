#pragma once

struct BigGunInfo
{
	EulerAngle Orientation;
	float BarrelZRotation;
	float StartYRot;
	float GunRotYAdd;

	unsigned int FireCount;
	bool BarrelRotating;

	char Flags;
};
