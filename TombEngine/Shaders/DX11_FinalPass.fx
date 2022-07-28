#include "./VertexInput.hlsli"

cbuffer PostProcessBuffer : register(b7)
{
	float CinematicBarsHeight;
	float ScreenFadeFactor;
	int ViewportWidth;
	int ViewportHeight;
};

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

	float y = input.Position.y / ViewportHeight;

	if (y > 1.0f - CinematicBarsHeight ||
		y < 0.0f + CinematicBarsHeight)
	{
		output = float4(0, 0, 0, 1);
	}
	else
	{
		output.xyz = output.xyz * colorMul.xyz * ScreenFadeFactor;
		output.w = 1.0f;
	}

	return output;
}