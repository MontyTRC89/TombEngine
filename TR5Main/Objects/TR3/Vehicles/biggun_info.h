#pragma once

struct BigGunInfo
{
	EulerAngles Orientation;
	float BarrelZRotation;
	float StartYRot;
	float GunRotYAdd;

	unsigned int FireCount;
	bool BarrelRotating;

	char Flags;
};
