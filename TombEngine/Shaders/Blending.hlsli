#ifndef BLENDINGSHADER
#define BLENDINGSHADER

#include "./Math.hlsli"

#define ALPHA_TEST_NONE			0
#define ALPHA_TEST_GREATER_THAN 1
#define ALPHA_TEST_LESS_THAN	2

#define BLENDMODE_OPAQUE	  0,
#define BLENDMODE_ALPHATEST	  1
#define BLENDMODE_ADDITIVE	  2
#define BLENDMODE_NOZTEST	  4
#define BLENDMODE_SUBTRACTIVE 5
#define BLENDMODE_WIREFRAME	  6
#define BLENDMODE_EXCLUDE	  8
#define BLENDMODE_SCREEN	  9
#define BLENDMODE_LIGHTEN	  10
#define BLENDMODE_ALPHABLEND  11

#define ZERO	   float3(0.0f, 0.0f, 0.0f)
#define EIGHT_FIVE float3( 0.85f, 0.85f, 0.85f)
#define BLENDING   0.707f

cbuffer BlendingBuffer : register(b12)
{
	uint  BlendMode;
	int	  AlphaTest;
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

float4 DoDistanceFogForPixel(float4 sourceColor, float4 fogColor, float value)
{
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

float4 DoFogBulbsForPixel(float4 sourceColor, float4 fogColor)
{
	switch (BlendMode)
	{
	case BLENDMODE_ADDITIVE:
	case BLENDMODE_SCREEN:
	case BLENDMODE_LIGHTEN:
		fogColor.xyz *= Luma(sourceColor);
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

	float4 result = sourceColor;

	result.xyz += saturate(fogColor.xyz);

	return result;
}

float4 DoLaserBarrierEffect(float3 input, float4 output, float2 uv, float faceFactor, float timeUniform)
{
	float2 noiseTexture = input.xy / uv;
	noiseTexture *= uv.x / uv.y;
	float noiseValue = FractalNoise(noiseTexture * 8.0f - timeUniform);

	float4 color = output;
	float gradL = smoothstep(0.0, 1.0, uv.x);
	float gradR = smoothstep(1.0, 0.0, uv.x);
	float gradT = smoothstep(0.0, 0.25, uv.y);
	float gradB = 1.0 - smoothstep(0.75, 1.0, uv.y);

	float distortion = timeUniform / 1024;

	float3 noisix = SimplexNoise
	(
		SimplexNoise(float3(input.r * distortion, input.g, input.b))
	);
	float3 shadowx = SimplexNoise
	(
		cos(SimplexNoise(sin(timeUniform + input.rgb * 400)))
	);

	noisix.x = noisix.x > 0.9 ? 0.7 : noisix.x;
	noisix.y = noisix.y > 0.9 ? 0.7 : noisix.y;
	noisix.z = noisix.z > 0.9 ? 0.7 : noisix.z;
	color.rgb *= noisix + 1.3f;
	color.rgb -= noisix + 0.2f;

	float frequency = 0.1;
	float amplitude = 0.8;
	float persistence = 0.5;
	float noiseValue2 = 0;
	float noiseValue3 = 0;

	float2 uv84 = (uv * 2.4);
	uv84.y = (uv84.y - 1.3);
	uv84.x = (uv84.x / 1.3);
	float2 uv85 = (uv / 2.4);

	noiseValue2 = AnimatedNebula(uv84, timeUniform * 0.1f);

	frequency = 2.5;
	amplitude = 0.2;
	persistence = 4.7;

	float2 uv83 = uv * 8;
	uv83.y = (uv.y + (timeUniform * 0.02));
	noiseValue3 = NebularNoise(uv83, frequency, amplitude, persistence);

	noiseValue2 += AnimatedNebula(uv/2, timeUniform * 0.05f);
	
	color.rgb -= noiseValue - 0.7f;
	color.rgb *= noiseValue2 + 1.0f;
	color.rgb += noiseValue3;
	color.a *= noiseValue + 0.01f;

	color.rgb -= shadowx + 0.1f;

	color.a *= noiseValue2 + 0.9f;
	color.a *= noiseValue3 + 2.0f;

	float fade0 = faceFactor * max(0.0, 1.0 - dot(float2(BLENDING, BLENDING), float2(gradL, gradT)));
	float fade1 = faceFactor * max(0.0, 1.0 - dot(float2(BLENDING, BLENDING), float2(gradL, gradB)));
	float fade2 = faceFactor * max(0.0, 1.0 - dot(float2(BLENDING, BLENDING), float2(gradR, gradB)));
	float fade3 = faceFactor * max(0.0, 1.0 - dot(float2(BLENDING, BLENDING), float2(gradR, gradT)));

	float fadeL = 1.40f * faceFactor * faceFactor * (1.0 - gradL);
	float fadeB = 2.75f * faceFactor * faceFactor * (1.0 - gradB);
	float fadeR = 1.40f * faceFactor * faceFactor * (1.0 - gradR);
	float fadeT = 2.75f * faceFactor * faceFactor * (1.0 - gradT);

	float fade = max(
		max(max(fade0, fade1), max(fade2, fade3)),
		max(max(fadeL, fadeR), max(fadeB, fadeT)));

	float scale = 1.0 - fade;

	color *= scale;
	float decayFactor = 1.0f;
	if (uv.y > 0.5f && uv.y < 1.0f)
	{
		decayFactor = uv.y / 2;
	}
	if (uv.y < 0.5f && uv.y > 0.0f)
	{
		decayFactor = (1.0f - uv.y) / 2;
	}
	color *= decayFactor;

	color.rgb = smoothstep(ZERO, EIGHT_FIVE, color.rgb);
	return color;
}

#endif // BLENDINGSHADER
