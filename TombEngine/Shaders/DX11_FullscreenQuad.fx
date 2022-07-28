#include "./VertexInput.hlsli"

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	output.Position = float4(input.Position, 1.0f);
	output.Color = input.Color;
	output.UV = input.UV;

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	float3 colorMul = min(input.Color.xyz, 1.0f);
	output.xyz = output.xyz * colorMul.xyz;
	//output.w = 1.0f;

	return output;
}