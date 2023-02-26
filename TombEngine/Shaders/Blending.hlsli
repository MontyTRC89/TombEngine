#ifndef BLENDINGSHADER
#define BLENDINGSHADER

#include "./Math.hlsli"

#define ALPHA_TEST_NONE 0
#define ALPHA_TEST_GREATER_THAN 1
#define ALPHA_TEST_LESS_THAN 2

#define BLENDMODE_OPAQUE 0,
#define BLENDMODE_ALPHATEST 1
#define BLENDMODE_ADDITIVE 2
#define BLENDMODE_NOZTEST 4
#define BLENDMODE_SUBTRACTIVE 5
#define BLENDMODE_WIREFRAME 6
#define BLENDMODE_EXCLUDE 8
#define BLENDMODE_SCREEN 9
#define BLENDMODE_LIGHTEN 10
#define BLENDMODE_ALPHABLEND 11

cbuffer BlendingBuffer : register(b12)
{
	uint BlendMode;
	int AlphaTest;
	float AlphaThreshold;
};

void DoAlphaTest(float4 inputColor)
{
	if (AlphaTest == ALPHA_TEST_GREATER_THAN && inputColor.w < AlphaThreshold)
	{
		discard;
	}
	else if (AlphaTest == ALPHA_TEST_LESS_THAN && inputColor.w > AlphaThreshold)
	{
		discard;
	}
	else
	{
		return;
	}
}

float4 DoFog(float4 sourceColor, float4 fogColor, float value)
{
	if (FogMaxDistance == 0)
		return sourceColor;

	switch (BlendMode)
	{
		case BLENDMODE_ADDITIVE:
		case BLENDMODE_SCREEN:
		case BLENDMODE_LIGHTEN:
			fogColor.xyz *= Luma(sourceColor.xyz);
			break;

		case BLENDMODE_SUBTRACTIVE:
		case BLENDMODE_EXCLUDE:
			fogColor.xyz *= 1.0f - Luma(sourceColor.xyz);
			break;

		case BLENDMODE_ALPHABLEND:
			fogColor.w = sourceColor.w;
			break;

		default:
			break;
	}

	if (fogColor.w > sourceColor.w)
		fogColor.w = sourceColor.w;
	
	float4 result = lerp(sourceColor, fogColor, value);
	return result;
}

#endif // BLENDINGSHADER