#include "./CameraMatrixBuffer.hlsli"
#include "./Blending.hlsli"
#include "./VertexInput.hlsli"
#include "./Math.hlsli"

// NOTE: This shader is used for all opaque or not sorted transparent sprites, that can be instanced for a faster drawing

#define INSTANCED_SPRITES_BUCKET_SIZE 512

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float2 UV: TEXCOORD1;
	float4 Color: COLOR;
	float Fog : FOG;
	float4 PositionCopy: TEXCOORD2;
};

struct InstancedSprite
{
	float4x4 World;
	float4 UV[2];
	float4 Color;
	float IsBillboard;
	float IsSoftParticle;
};

cbuffer InstancedSpriteBuffer : register(b13)
{
	InstancedSprite Sprites[INSTANCED_SPRITES_BUCKET_SIZE];
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D DepthMap : register(t6);
SamplerState DepthMapSampler : register(s6);

PixelShaderInput VS(VertexShaderInput input, uint InstanceID : SV_InstanceID)
{
	PixelShaderInput output;

	float4 worldPosition;

	if (Sprites[InstanceID].IsBillboard == 1)
	{
		worldPosition = mul(float4(input.Position, 1.0f), Sprites[InstanceID].World);
		output.Position = mul(mul(float4(input.Position, 1.0f), Sprites[InstanceID].World), ViewProjection);
	}
	else
	{
		worldPosition = float4(input.Position, 1.0f);
		output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
	}

	output.PositionCopy = output.Position;
	output.Color = Sprites[InstanceID].Color;
	output.UV = float2(Sprites[InstanceID].UV[0][input.PolyIndex], Sprites[InstanceID].UV[1][input.PolyIndex]);

	float4 d = length(CamPositionWS - worldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);

	return output;
}

// TODO: From NVIDIA SDK, check if it can be useful instead of linear ramp
float Contrast(float Input, float ContrastPower)
{
#if 1
	//piecewise contrast function
	bool IsAboveHalf = Input > 0.5;
	float ToRaise = saturate(2 * (IsAboveHalf ? 1 - Input : Input));
	float Output = 0.5 * pow(ToRaise, ContrastPower);
	Output = IsAboveHalf ? 1 - Output : Output;
	return Output;
#else
	// another solution to create a kind of contrast function
	return 1.0 - exp2(-2 * pow(2.0 * saturate(Input), ContrastPower));
#endif
}

float4 PS(PixelShaderInput input, uint InstanceID : SV_InstanceID) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV) * input.Color;

	//DoAlphaTest(output);

	if (Sprites[InstanceID].IsSoftParticle == 1)
	{
		float particleDepth = input.PositionCopy.z / input.PositionCopy.w;
		input.PositionCopy.xy /= input.PositionCopy.w;
		float2 texCoord = 0.5f * (float2(input.PositionCopy.x, -input.PositionCopy.y) + 1);
		float sceneDepth = DepthMap.Sample(DepthMapSampler, texCoord).r;

		sceneDepth = LinearizeDepth(sceneDepth, NearPlane, FarPlane);
		particleDepth = LinearizeDepth(particleDepth, NearPlane, FarPlane);

		if (particleDepth - sceneDepth > 0.01f)
			discard;

		float fade = (sceneDepth - particleDepth) * 1024.0f;
		output.w = min(output.w, fade);
	}

	output = DoFog(output, float4(0.0f, 0.0f, 0.0f, 0.0f), input.Fog);

	return output;
}