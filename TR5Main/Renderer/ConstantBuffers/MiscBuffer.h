#pragma once

struct alignas(16) CMiscBuffer
{
	int AlphaTest;
	int Caustics;
	float Padding[14];
};