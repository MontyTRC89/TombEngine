#include "./CameraMatrixBuffer.hlsli"
#include "./AlphaTestBuffer.hlsli"
#include "./VertexInput.hlsli"

cbuffer ItemBuffer : register(b1)
{
	float4x4 World;
	float4x4 Bones[32];
	float4 ItemPosition;
	float4 AmbientLight;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float3 WorldPosition : POSITION;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	output.Position = mul(mul(float4(input.Position, 1.0f), World), ViewProjection);
	output.Normal = (mul(float4(input.Normal, 0.0f), World).xyz);
	output.Color = input.Color;
	output.UV = input.UV;
	output.WorldPosition = (mul(float4(input.Position, 1.0f), World));

	return output;
}

[earlydepthstencil]
float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);

	DoAlphaTest(output);

	float4 lightDirection = float4(-1.0f, 0.707f, -1.0f, 1.0f);
	float4 lightColor = float4(1.0f, 1.0f, 0.5f, 1.0f);
	float lightIntensity = 0.6f;
	float d = max(dot(-lightDirection, input.Normal), 0.0f);
	float4 light = AmbientLight + d * lightColor * lightIntensity;

	output.xyz *= light.xyz;
	output.w = 1.0f;

	return output;
}