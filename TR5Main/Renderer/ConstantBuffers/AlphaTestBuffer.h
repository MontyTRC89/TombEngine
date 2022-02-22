#pragma once

struct alignas(16) CAlphaTestBuffer
{
	int AlphaTest;
	float AlphaThreshold;
	float Padding[14];
};