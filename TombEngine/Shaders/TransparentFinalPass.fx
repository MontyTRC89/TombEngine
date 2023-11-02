#include "./VertexInput.hlsli"
#include "./Math.hlsli"

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float2 UV: TEXCOORD0;
	float4 Color: COLOR0;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D TransparentTexture : register(t1);
SamplerState TransparentSampler : register(s1);

Texture2D WeightTexture : register(t2);
SamplerState WeightSampler : register(s2);

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
	float4 transparent = TransparentTexture.Sample(TransparentSampler, input.UV);
	float weight = WeightTexture.Sample(WeightSampler, input.UV).x;

	return float4(transparent.xyz / clamp(transparent.w, 0.0001f, 5000.0f), 1.0f - weight);
}