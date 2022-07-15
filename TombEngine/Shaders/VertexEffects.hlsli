float3 Glow(float3 color, float3 effect, int hash)
{
	float3 col = color;
	float wibble = sin((((Frame + hash) % 256) / 256.0) * (PI2)); // sin from -1 to 1 with a period of 64 frames

	if (effect.x > 0.0f)
	{
		float intensity = effect.x * lerp(-0.5f, 1.0f, wibble * 0.5f + 0.5f);
		col = saturate(color + float3(intensity, intensity, intensity));
	}

	return col;
}

float3 Move(float3 position, float3 effect, int hash)
{
	float3 pos = position;
	float weight = effect.z;
	float wibble = sin((((Frame + hash) % 256) / 256.0) * (PI2)); // sin from -1 to 1 with a period of 64 frames

	if (effect.y > 0.0f)
		pos.y += wibble * effect.y * weight * 128.0f; // 128 units offset to top and bottom (256 total)

	return pos;
}