#include "./CameraMatrixBuffer.hlsli"
#include "./AlphaTestBuffer.hlsli"
#include "./VertexInput.hlsli"

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
	output.Normal = input.Normal;
	output.Color = input.Color;
	output.UV = input.UV;

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	
	DoAlphaTest(output);

	float3 colorMul = min(input.Color.xyz, 1.0f) * 2.0f;
	output.xyz = output.xyz * colorMul.xyz;
	output.w = 1.0f;

	return output;
}