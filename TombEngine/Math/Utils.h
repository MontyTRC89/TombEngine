#pragma once

namespace TEN::Math
{
	constexpr inline auto OFFSET_RADIUS = [](auto x) { return ((x * SQRT_2) + 4); };
	constexpr inline auto MESH_BITS		= [](auto x) { return (1 << x); };

	// Value manipulation

	float FloorToStep(float value, float step);
	float CeilToStep(float value, float step);
	float RoundToStep(float value, float step);
	float Remap(float value, float min0, float max0, float min1, float max1);

	// Interpolation

	float Lerp(float value0, float value1, float alpha);
	float Smoothstep(float value0, float value1, float alpha);
	float Smoothstep(float alpha);
	float EaseInSine(float value0, float value1, float alpha);
	float EaseInSine(float alpha);
	float EaseOutSine(float value0, float value1, float alpha);
	float EaseOutSine(float alpha);
	float EaseInOutSine(float value0, float value1, float alpha);
	float EaseInOutSine(float alpha);

	// Color

	float	Luma(const Vector3& color);
	Vector3 Screen(const Vector3& ambient, const Vector3& tint);
	Vector4 Screen(const Vector4& ambient, const Vector4& tint);
}

namespace std
{
	template <>
	struct hash<Plane>
	{
		size_t operator()(const Plane& plane) const
		{
			size_t seed = 0;
			seed ^= hash<float>()(plane.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= hash<float>()(plane.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= hash<float>()(plane.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= hash<float>()(plane.w) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}
	};

	template <>
	struct hash<Vector3>
	{
		size_t operator ()(const Vector3& vector) const noexcept
		{
			size_t seed = 0;
			seed ^= hash<float>()(vector.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= hash<float>()(vector.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= hash<float>()(vector.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}
	};
}
