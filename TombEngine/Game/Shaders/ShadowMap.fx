#include "./CBCamera.hlsli"
#include "./Blending.hlsli"
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
	float4 PositionCopy : TEXCOORD1;
	float Depth: TEXCOORD2;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4x4 world = mul(Bones[input.Bone], World);

	output.Position = mul(mul(float4(input.Position, 1.0f), world), ViewProjection);
	output.Depth = output.Position.z / output.Position.w;
	output.PositionCopy = output.Position;

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	return float4(input.PositionCopy.z / input.PositionCopy.w, 0, 0, 0);
}