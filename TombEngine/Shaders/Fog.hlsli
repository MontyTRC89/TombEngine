#ifndef FOGSHADER
#define FOGSHADER

float4 ApplyFog(float4 sourceColor, float4 fogColor, float value, unsigned int blendMode)
{
	if (FogMaxDistance == 0)
		return sourceColor;
	
	if (blendMode != 0 &&
		blendMode != 1 &&
		blendMode != 4 &&
		blendMode != 6 &&
		blendMode != 11)
	{
		fogColor.w = 0.0f;
	}

	if (fogColor.w > sourceColor.w)
	{
		fogColor.w = sourceColor.w;
	}
	
	return lerp(sourceColor, fogColor, value);
}

#endif // FOGSHADER