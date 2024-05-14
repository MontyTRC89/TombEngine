#pragma once

namespace TEN::Math
{
	constexpr inline auto OFFSET_RADIUS = [](auto x) { return ((x * SQRT_2) + 4); };
	constexpr inline auto MESH_BITS		= [](auto x) { return (1 << x); };

	// Interpolation
	float Lerp(float value0, float value1, float alpha);
	float InterpolateCos(float value0, float value1, float alpha);
	float InterpolateCubic(float value0, float value1, float value2, float value3, float alpha);
	float Smoothstep(float alpha);
	float Smoothstep(float value0, float value1, float alpha);

	// Color
	float Luma(const Vector3& color);
	Vector3 Screen(const Vector3& ambient, const Vector3& tint);
	Vector4 Screen(const Vector4& ambient, const Vector4& tint);

	// Misc.
	float FloorToStep(float value, float step);
	float CeilToStep(float value, float step);
	float RoundToStep(float value, float step);
}
