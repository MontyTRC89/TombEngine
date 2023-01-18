#pragma once

namespace TEN::Math
{
	const float Lerp(float value0, float value1, float alpha);
	const float InterpolateCos(float value0, float value1, float alpha);
	const float InterpolateCubic(float value0, float value1, float value2, float value3, float alpha);
	const float Smoothstep(float alpha);
	const float Smoothstep(float value0, float value1, float alpha);
}
