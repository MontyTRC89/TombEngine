#ifndef MATH
#define MATH

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923
#define PI2 6.2831853071795864769252867665590057683943387987502116419498891846

float Luma(float3 color)
{
	// Use Rec.709 trichromat formula to get perceptive luma value
	return float((color.x * 0.2126f) + (color.y * 0.7152f) + (color.z * 0.0722f));
}

float3 Screen(float3 ambient, float3 tint)
{
	float luma = Luma(tint);

	float3 multiplicative = ambient * tint;
	float3 additive = ambient + tint;

	float r = lerp(multiplicative.x, additive.x, luma);
	float g = lerp(multiplicative.y, additive.y, luma);
	float b = lerp(multiplicative.z, additive.z, luma);

	return float3(r, g, b);
}

#endif // MATH