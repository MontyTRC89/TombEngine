#pragma once

struct alignas(16) CStaticBuffer
{
	Matrix World;
	Vector4 Position;
	Vector4 Color;
	int LightMode;
};
