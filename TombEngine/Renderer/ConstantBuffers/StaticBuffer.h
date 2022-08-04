#pragma once

struct alignas(16) CStaticBuffer
{
	Matrix World;
	Vector4 Color;
	Vector4 AmbientLight;
	int LightMode;
};
