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

#define ZERO float3(.0f,.0f,.0f)
#define EIGHT_FIVE float3(.85f,.85f,.85f)
#define BLENDING .707f

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

float4 DoLasers(float3 input, float4 output, float2 UV, float fade_factor, float timeUniform)
{
	float2 noiseTexture = (input.xy) / (UV );
	noiseTexture *= (UV.x ) / (UV.y );
	float noiseValue = FractalNoise(noiseTexture * 8.f - timeUniform);

	float4 color = output;
	float gradL = smoothstep(0.0, 1.0, UV.x);
	float gradR = smoothstep(1.0, 0.0, UV.x);
	float gradT = smoothstep(0.0, 1.0, UV.y);
	float gradB =  smoothstep(1.0, 0.0, UV.y);

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
	color.rgb -= noisix + .2f;

	float frequency = 0.1;
	float amplitude = 0.8;
	float persistence = 0.5;
	float noiseValue2 = 0;
	float noiseValue3 = 0;

	float2 uv84 = (UV * 2.4);
	uv84.y = (uv84.y - 1.3);
	uv84.x = (uv84.x / 1.3);
	float2 uv85 = (UV / 2.4);

	noiseValue2 = AnimatedNebula(uv84, timeUniform * 0.1f);

	frequency = 2.5;
	amplitude = 0.2;
	persistence = 4.7; 

	float2 uv83 = UV * 8;
	uv83.y = (UV.y + (timeUniform * 0.02));
	noiseValue3 = NebularNoise(uv83, frequency, amplitude, persistence);

	noiseValue2 += AnimatedNebula(UV/2, timeUniform * 0.05f);
	
	color.a *= noiseValue + .01f;
	color.rgb -= noiseValue - 0.7f;
	color.rgb *= noiseValue2 + 1.0f;
	color.rgb += noiseValue3;

	color.rgb -= shadowx + 0.1f;

	color.a *= noiseValue2 + 0.9f;
	color.a *= noiseValue3 + 2.0f;

	float fade0 = fade_factor * max(0.0, 1.0 - dot(float2(BLENDING, BLENDING), float2(gradL, gradT)));
	float fade1 = fade_factor * max(0.0, 1.0 - dot(float2(BLENDING, BLENDING), float2(gradL, gradB)));
	float fade2 = fade_factor * max(0.0, 1.0 - dot(float2(BLENDING, BLENDING), float2(gradR, gradB)));
	float fade3 = fade_factor * max(0.0, 1.0 - dot(float2(BLENDING, BLENDING), float2(gradR, gradT)));

	float fadeL = 1.40f * fade_factor * fade_factor * (1.0 - gradL);
	float fadeB =  fade_factor * fade_factor * (1.0 - gradB);
	float fadeR = 1.40f * fade_factor * fade_factor * (1.0 - gradR);
	float fadeT = fade_factor * fade_factor * (1.0 - gradT);

	float fade = max(
		max(max(fade0, fade1), max(fade2, fade3)),
		max(max(fadeL, fadeR), max(fadeB, fadeT)));

	float scale = 1.0 - fade;

	color *= scale;
	float decayFactor = 1;
	if (UV.y > .5f && UV.y < 1)
	{
		decayFactor = UV.y / 2;
	}
	if (UV.y < .5f && UV.y>0)
	{
		decayFactor = (1 - UV.y) / 2;
	}
	color *= decayFactor;

	color.rgb = smoothstep(ZERO, EIGHT_FIVE, color.rgb);

	return color;
}

#endif // BLENDINGSHADER
