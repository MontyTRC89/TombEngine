#pragma once

struct alignas(16) CBlendingBuffer
{
	unsigned int BlendMode;
	int AlphaTest;
	float AlphaThreshold;
};