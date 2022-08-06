#pragma once

struct alignas(16) CItemBuffer
{
	Matrix World;
	Matrix BonesMatrices[MAX_BONES];
	Vector4 Color;
	Vector4 AmbientLight;
	int BoneLightModes[MAX_BONES];
};
