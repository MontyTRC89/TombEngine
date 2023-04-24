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
	float2 noiseTexture = input.xy/UV;
	noiseTexture *=  UV.x/UV.y;
	float noiseValue = fractalNoise(noiseTexture*3.f-timeUniform);

	float4 color = output;
	float gradL = smoothstep(0.0, 0.25, UV.x);
  float gradR = 1.0 - smoothstep(0.75, 1.0, UV.x);
  float gradT = smoothstep(0.0, 0.25, UV.y);
  float gradB = 1.0 - smoothstep(0.75, 1.0, UV.y);
  
	float distortion = timeUniform/1024;

  float3 noisix = SimplexNoise
	(
		SimplexNoise(float3(input.r*distortion,input.g,input.b))
	);
	float3 shadowx = SimplexNoise
	(
		cos(SimplexNoise(sin(timeUniform+input.rgb*200)))
	);

  noisix.x = noisix.x > 0.9 ? 0.1 : noisix.x;
  noisix.y = noisix.y > 0.9 ? 0.1 : noisix.y;
  noisix.z = noisix.z > 0.9 ? 0.1 : noisix.z;
  color.rgb *= noisix;
  color.rgb -= noisix;
  
  float fade0 = fade_factor * max(0.0, 1.0 - dot(float2(BLENDING,BLENDING), float2(gradL, gradT)));
  float fade1 = fade_factor * max(0.0, 1.0 - dot(float2(BLENDING,BLENDING), float2(gradL, gradB)));
  float fade2 = fade_factor * max(0.0, 1.0 - dot(float2(BLENDING,BLENDING), float2(gradR, gradB)));
  float fade3 = fade_factor * max(0.0, 1.0 - dot(float2(BLENDING,BLENDING), float2(gradR, gradT)));
  
  float fadeL = fade_factor * fade_factor * (1.0 - gradL);
  float fadeB = 1.75f*fade_factor * fade_factor * (1.0 - gradB);
  float fadeR = fade_factor * fade_factor * (1.0 - gradR);
  float fadeT = 1.75f*fade_factor * fade_factor * (1.0 - gradT);
  
  float fade = max(
    max(max(fade0, fade1), max(fade2, fade3)),
    max(max(fadeL, fadeR), max(fadeB, fadeT)));
    
  float scale = 1.0 - fade; 
  
  color *= scale;
	float decayFactor = 1;
	if(UV.y>.5f && UV.y<.75f)
	{
		decayFactor = UV.y/2;
	}
	if(UV.y<.5f && UV.y>.25f)
	{
		decayFactor = (1-UV.y)/2;
	}

	//Initial decay factor for Lasers
	color *= decayFactor;
  color.rgb -= shadowx;
  color.rgb = smoothstep(ZERO,EIGHT_FIVE,color.rgb);

	//Add fractal noise effect
	color.a *= noiseValue+.1f;
	color.r *= 2*noiseValue;
	color.gb -= noiseValue;

	//Add shadow that moves diagonally
	noiseValue = 0;
	float2 uv8 = UV*8+timeUniform*0.1f;
	matrix <float, 2, 2> transformationMatrix = {1.6f,  0.6f, -1.2f,  1.6f};
	noiseValue  = 0.5000*Noise2D( uv8 ); uv8 = mul(uv8, transformationMatrix);
	noiseValue += 0.2500*Noise2D( uv8 ); uv8 = mul(uv8, transformationMatrix);
	noiseValue += 0.1250*Noise2D( uv8 ); uv8 = mul(uv8, transformationMatrix);
	noiseValue += 0.0625*Noise2D( uv8 ); uv8 = mul(uv8, transformationMatrix);
	color.rgb = max(color.rgb-noiseValue,0);

	return color;
}

#endif // BLENDINGSHADER
