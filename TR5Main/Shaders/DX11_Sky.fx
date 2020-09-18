#include "./CameraMatrixBuffer.hlsli"

cbuffer StaticMatrixBuffer : register(b1)
{
	float4x4 World;
	float4 StaticPosition;
	float4 Color;
};

cbuffer MiscBuffer : register(b3)
{
	int AlphaTest;
};

#include "./VertexInput.hlsli"

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};

Texture2D Texture;
SamplerState Sampler;

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	output.Position = mul(mul(float4(input.Position, 1.0f), World), ViewProjection);
	output.Normal = input.Normal;
	output.Color = input.Color;
	output.UV = input.UV;

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	output.xyz = output.xyz * Color;

	return output;
}