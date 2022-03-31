#pragma once

struct BigGunInfo
{
	PHD_3DPOS Rotation;
	short BarrelZRotation;
	short StartYRot;
	long GunRotYAdd;

	unsigned int FireCount;
	bool BarrelRotating;

	char Flags;
};
