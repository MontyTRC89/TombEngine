#ifndef FOGSHADER
#define FOGSHADER

#include "./Math.hlsli"

float4 ApplyFog(float4 sourceColor, float4 fogColor, float value, unsigned int blendMode)
{
	if (FogMaxDistance == 0)
		return sourceColor;

	float alphaMult = 1.0f;

	switch (blendMode)
	{
		case 2:
		case 9:
		case 10:
			alphaMult = Luma(sourceColor.xyz);
			fogColor.w = 0.0f;
			break;

		case 5:
		case 8:
			alphaMult = 1.0f - Luma(sourceColor.xyz);
			fogColor.w = 0.0f;
			break;

		case 11:
			alphaMult = sourceColor.w;
			break;

		default:
			break;
	}

	if (fogColor.w > sourceColor.w)
		fogColor.w = sourceColor.w;
	
	float4 result = lerp(sourceColor, fogColor, value);
	result.w *= alphaMult;
	return result;
}

#endif // FOGSHADER