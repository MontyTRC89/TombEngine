#pragma once

namespace TEN::Math
{
	float Lerp(float value0, float value1, float alpha);
	float InterpolateCos(float value0, float value1, float alpha);
	float InterpolateCubic(float value0, float value1, float value2, float value3, float alpha);
	float Smoothstep(float alpha);
	float Smoothstep(float value0, float value1, float alpha);
}
