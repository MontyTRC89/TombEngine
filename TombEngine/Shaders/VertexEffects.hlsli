#include "./Math.hlsli"

#define WIBBLE_FRAME_PERIOD 64.0f

float Wibble(float3 effect, int hash)
{
	if (effect.x > 0.0f || effect.y > 0.0f)
		return sin((((Frame + hash) % 256) / WIBBLE_FRAME_PERIOD) * (PI2));
	else
		return 0.0f; // Don't calculate if not necessary
}

float3 Glow(float3 color, float3 effect, float wibble)
{
	float3 col = color;

	if (effect.x > 0.0f)
	{
		float intensity = effect.x * lerp(-0.5f, 1.0f, wibble * 0.5f + 0.5f);
		col = color + float3(intensity, intensity, intensity);
	}

	return col;
}

float3 Move(float3 position, float3 effect, float wibble)
{
	float3 pos = position;
	float weight = effect.z;

	if (effect.y > 0.0f && weight > 0.0f)
	{
		pos.y += wibble * effect.y * weight * 128.0f; // 128 units offset to top and bottom (256 total)
	}

	return pos;
}