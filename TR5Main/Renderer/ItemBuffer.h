#pragma once

struct alignas(16) CItemBuffer
{
	Matrix World;
	Matrix BonesMatrices[32];
	Vector4 Position;
	Vector4 AmbientLight;
};

